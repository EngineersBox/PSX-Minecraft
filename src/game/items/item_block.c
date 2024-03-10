#include "item_block.h"

#include <psxgte.h>
#include <psxgpu.h>
#include <inline_c.h>

#include "../../structure/primitive/clip.h"
#include "../../math/math_utils.h"
#include "../../structure/primitive/cube.h"

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

void itemBlockRenderWorld(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {
    int p;
    TextureAttributes* face_attribute;
    const Texture* texture = &textures[TERRAIN_TEXTURES];
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
            item->item.position.vx + item_block_verts[CUBE_INDICES[i]._v].vx, \
            item->item.position.vy + item_block_verts[CUBE_INDICES[i]._v].vy, \
            item->item.position.vz + item_block_verts[CUBE_INDICES[i]._v].vz, \
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

void itemBlockRenderInventory(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {

}

void itemBlockRenderHand(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {

}
