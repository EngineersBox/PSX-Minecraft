#include "item_block.h"

#include <psxgte.h>
#include <psxgpu.h>
#include <inline_c.h>

#include "../../structure/primitive/clip.h"
#include "../../math/math_utils.h"
#include "../../structure/primitive/cube.h"
#include "../../resources/asset_indices.h"

#define VERTICES_COUNT 8
SVECTOR item_block_verts[VERTICES_COUNT] = {
    { -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 },
    {  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 },
    { -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 },
    {  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 },
    {  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 },
    { -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 },
    {  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 },
    { -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 }
};
#define ITEM_BLOCK_BOB_ANIM_SAMPLES 37
// Minecraft's item spin rate is 2.87675 degrees per tick
// 2.87675 / 360 = 0.0079909722
// (2.87675 / 360) * 4096 = 32.7310222222
#define ITEM_ROTATION_QUANTA 32

// Domain: [0,36] -> [0,1] (X)
// Range:  [0,16] (Y)
// f(x) = 16 * (1 / (1 + e^((-7.5 * x) + (7.5 / 2))))
const int32_t sigmoid_lut[ITEM_BLOCK_BOB_ANIM_SAMPLES] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
    2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9,
    10, 11, 12, 12, 13, 13, 13, 14,
    14, 14, 15, 15, 15, 15, 15
};
// Domain: [0,36] -> [0,1] (X)
// Range:  [0,16] (Y)
// f(x) = 16 * (0.5 + ((sin((pi * x) - (pi / 2))) / 2))
const int32_t sin_lut[ITEM_BLOCK_BOB_ANIM_SAMPLES] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
    3, 3, 4, 5, 5, 6, 6, 7, 8, 9, 9,
    10, 10, 11, 12, 12, 13, 13, 14,
    14, 14, 15, 15, 15, 15, 15
};

#ifndef ITEM_BLOCK_ANIM_LUT
#define ITEM_BLOCK_ANIM_LUT sin_lut
#endif

const VECTOR item_stack_render_offsets[5] = {
    [0] = (VECTOR) {0},
    [1] = (VECTOR) {
        .vx = 4,
        .vy = 4,
        .vz = 4,
    },
    [2] = (VECTOR) {
        .vx = -5,
        .vy = 2,
        .vz = 3,
    },
    [3] = (VECTOR) {
        .vx = 2,
        .vy = 5,
        .vz = -4,
    },
    [4] = (VECTOR) {
        .vx = -3,
        .vy = 3,
        .vz = -5,
    },
};

void renderItemBlock(ItemBlock* item, RenderContext* ctx, const VECTOR* position_offset) {
    int p;
    TextureAttributes* face_attribute;
    const Texture* texture = &textures[ASSET_TEXTURES_TERRAIN_INDEX];
    RECT tex_window;
    for (int i  = 0; i < BLOCK_FACES; i++) {
        face_attribute = &item->face_attributes[i];
        tex_window = (RECT) {
            face_attribute->u >> 3,
            face_attribute->v >> 3,
            face_attribute->w >> 3,
            face_attribute->h >> 3
        };
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
#define createVert(_v) (SVECTOR) { \
            item_block_verts[CUBE_INDICES[i]._v].vx + position_offset->vx, \
            item_block_verts[CUBE_INDICES[i]._v].vy + position_offset->vy, \
            item_block_verts[CUBE_INDICES[i]._v].vz + position_offset->vz, \
            0 \
        }
        SVECTOR current_verts[4] = {
            createVert(v0),
            createVert(v1),
            createVert(v2),
            createVert(v3)
        };
        gte_ldv3(
            &current_verts[0],
            &current_verts[1],
            &current_verts[2]
        );
        // Rotation, Translation and Perspective Triple
        gte_rtpt();
        gte_nclip();
        gte_stopz(&p);
        // Avoid negative depth (behind camera) and zero
        // for constraint clearing primitive in OT
        if (p <= 0) {
            freePrimitive(ctx, sizeof(POLY_FT4));
            continue;
        }
        // Average screen Z result for three vertices
        gte_avsz3();
        gte_stotz(&p);
        if (p <= 0 || p >= ORDERING_TABLE_LENGTH) {
            freePrimitive(ctx, sizeof(POLY_FT4));
            continue;
        }
        // Initialize a textured quad primitive
        setPolyFT4(pol4);
        // Set the projected vertices to the primitive
        gte_stsxy0(&pol4->x0);
        gte_stsxy1(&pol4->x1);
        gte_stsxy2(&pol4->x2);
        // Compute the last vertex and set the result
        gte_ldv0(&current_verts[3]);
        gte_rtps();
        gte_stsxy(&pol4->x3);
        // Test if quad is off-screen, discard if so
        if (quadClip(
            &ctx->screen_clip,
            (DVECTOR*) &pol4->x0,
            (DVECTOR*) &pol4->x1,
            (DVECTOR*) &pol4->x2,
            (DVECTOR*) &pol4->x3)) {
            freePrimitive(ctx, sizeof(POLY_FT4));
            continue;
            }
        // Load primitive color even though gte_ncs() doesn't use it.
        // This is so the GTE will output a color result with the
        // correct primitive code.
        if (face_attribute->tint.cd) {
            setRGB0(
                pol4,
                face_attribute->tint.r,
                face_attribute->tint.g,
                face_attribute->tint.b
            );
        }
        gte_ldrgb(&pol4->r0);
        // Load the face normal
        gte_ldv0(&CUBE_NORMS[i]);
        // Apply RGB tinting to lighting calculation result on the basis
        // that it is enabled.
        if (face_attribute->tint.cd) {
            // Normal Color Column Single
            gte_nccs();
        } else {
            // Normal Color Single
            gte_ncs();
        }
        // Store result to the primitive
        gte_strgb(&pol4->r0);
        // Set texture coords and dimensions
        setUVWH(
            pol4,
            face_attribute->u,
            face_attribute->v,
            face_attribute->w,
            face_attribute->h
        );
        // Bind texture page and colour look-up-table
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        // Sort primitive to the ordering table
        uint32_t* ot_object = allocateOrderingTable(ctx, p);
        addPrim(ot_object, pol4);
        // Advance to make another primitive
        // Bind a texture window to ensure wrapping across merged block face primitives
        DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
        setTexWindow(ptwin, &tex_window);
        ot_object = allocateOrderingTable(ctx, p);
        addPrim(ot_object, ptwin);
    }
}

void itemBlockRenderWorld(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {
    VECTOR position = {
        .vx = item->item.position.vx,
        .vy = item->item.position.vy + ITEM_BLOCK_ANIM_LUT[item->item.bob_offset],
        .vz = item->item.position.vz
    };
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&item->item.rotation, &omtx);
    ApplyMatrixLV(&omtx, &transforms->translation_position, &transforms->translation_position);
    TransMatrix(&omtx, &position);
    // Multiply light matrix to object matrix
    MulMatrix0(&transforms->lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&transforms->geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    if (item->item.stack_size <= 1) {
        renderItemBlock(item, ctx, &item_stack_render_offsets[0]);
    } else if (item->item.stack_size <= 16) {
        #pragma GCC unroll 2
        for (int i = 0; i < 2; i ++) {
            renderItemBlock(item, ctx, &item_stack_render_offsets[i]);
        }
    } else if (item->item.stack_size <= 32) {
        #pragma GCC unroll 3
        for (int i = 0; i < 3; i ++) {
            renderItemBlock(item, ctx, &item_stack_render_offsets[i]);
        }
    } else if (item->item.stack_size <= 48) {
        #pragma GCC unroll 4
        for (int i = 0; i < 4; i ++) {
            renderItemBlock(item, ctx, &item_stack_render_offsets[i]);
        }
    } else {
        #pragma GCC unroll 5
        for (int i = 0; i < 5; i ++) {
            renderItemBlock(item, ctx, &item_stack_render_offsets[i]);
        }
    }
    item->item.rotation.vy = (item->item.rotation.vy + ITEM_ROTATION_QUANTA) % ONE;
    if (item->item.bob_offset <= 0) {
        item->item.bob_direction = 1;
    } else if (item->item.bob_offset >= ITEM_BLOCK_BOB_ANIM_SAMPLES) {
        item->item.bob_direction = -1;
    }
    item->item.bob_offset += item->item.bob_direction;
    PopMatrix();
}

void itemBlockRenderInventory(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {

}

void itemBlockRenderHand(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {

}
