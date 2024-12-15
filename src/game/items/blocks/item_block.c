#include "item_block.h"

#include <font.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <inline_c.h>

#include "../../../structure/primitive/clip.h"
#include "../../../structure/primitive/cube.h"
#include "../../../resources/asset_indices.h"
#include "../../../resources/assets.h"
#include "../../../logging/logging.h"
#include "../../gui/slot.h"
#include "../../../lighting/lightmap.h"

FWD_DECL typedef struct World World;
FWD_DECL LightLevel worldGetInternalLightLevel(const World* world);
FWD_DECL LightLevel worldGetLightValue(const World* world, const VECTOR* position);

#define VERTICES_COUNT 8
SVECTOR item_block_verts[VERTICES_COUNT] = {
    [0] = vec3_i16(-ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE), // 0b000
    [1] = vec3_i16( ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE), // 0b100
    [2] = vec3_i16(-ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE), // 0b010
    [3] = vec3_i16( ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE), // 0b110
    [4] = vec3_i16(-ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE), // 0b001
    [5] = vec3_i16( ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE), // 0b101
    [6] = vec3_i16(-ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE), // 0b011
    [7] = vec3_i16( ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE), // 0b111
};

// Domain: [0,36] -> [0,1] (X)
// Range:  [0,16] (Y)
// f(x) = 16 * (1 / (1 + e^((-7.5 * x) + (7.5 / 2))))
const i32 item_block_anim_sigmoid_lut[ITEM_BLOCK_BOB_ANIM_SAMPLES] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
    2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9,
    10, 11, 12, 12, 13, 13, 13, 14,
    14, 14, 15, 15, 15, 15, 15
};
// Domain: [0,36] -> [0,1] (X)
// Range:  [0,16] (Y)
// f(x) = 16 * (0.5 + ((sin((pi * x) - (pi / 2))) / 2))
const i32 item_block_anim_sin_lut[ITEM_BLOCK_BOB_ANIM_SAMPLES] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
    3, 3, 4, 5, 5, 6, 6, 7, 8, 9, 9,
    10, 10, 11, 12, 12, 13, 13, 14,
    14, 14, 15, 15, 15, 15, 15
};

const VECTOR item_stack_render_offsets[5] = {
    [0] = vec3_i32_all(0),
    [1] = vec3_i32(4,4,4),
    [2] = vec3_i32(-5,2,3),
    [3] = vec3_i32(2,5,-4),
    [4] = vec3_i32(-3,3,-5),
};

const u8 FULL_BLOCK_FACE_INDICES[FULL_BLOCK_FACE_INDICES_COUNT] = {0, 1, 2, 3, 4, 5};
const u8 ISOMETRIC_BLOCK_FACE_INDICES[ISOMETRIC_BLOCK_FACE_INDICES_COUNT] = {1, 3, 5};

/**
 * Why does this work? Heres the layout of the vertices explicitly:
 * { -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 }, // 0b000
 * {  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 }, // 0b100
 * { -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 }, // 0b010
 * {  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE, 0 }, // 0b110
 * { -ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 }, // 0b001
 * {  ITEM_BLOCK_SIZE, -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 }, // 0b101
 * { -ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 }, // 0b011
 * {  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE,  ITEM_BLOCK_SIZE, 0 }, // 0b111
 * Look familiar? Yeah.. that's a 3-bit binary adder output table.
 * So what we need is to take some 3-bit number and create the relevant
 * sign for the size (ITEM_BLOCK_SIZE in this case). We do that by
 * looking at the binary representation, which maps to each X,Y,Z
 * value's sign. So we can use a bit AND operation (&) to extract
 * the sign of each X,Y,Z value. That sign is either 0 or 1 (Encoded
 * as [0,1] here). We need a positive or negative value, so we need
 * a function to map [0,1] to [-1,1] which looks like this:
 * f(x) = -1 + (x << 1) <- Use x = 0 or x = 1 to see for yourself.
 * We can extend this with the bit encoding from before with masking
 * and shifting to get the sign bit (in the least-signficiant bit
 * position) as the value used with f(x) from before. This yields
 * the definition below, with the addition of a multiplier for the
 * desired vertex offset as size here.
 */

// [0,1] -> [-SIZE,SIZE]
#define convertToVertex(v, shift, size) (size * (-1 + ((((v) & (1 << (shift))) >> (shift)) << 1)))

void renderItemBlock(ItemBlock* item,
                     const u16 light_level_colour_scalar,
                     RenderContext* ctx,
                     const VECTOR* position_offset) {
    int p;
    TextureAttributes* face_attribute;
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    for (int i = 0; i < BLOCK_FACES; i++) {
        face_attribute = &item->face_attributes[i];
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        #define createVert(_v) vec3_i16( \
            convertToVertex(CUBE_INDICES[i]._v, 0, ITEM_BLOCK_SIZE) + position_offset->vx, \
            convertToVertex(CUBE_INDICES[i]._v, 1, ITEM_BLOCK_SIZE) - position_offset->vy, \
            convertToVertex(CUBE_INDICES[i]._v, 2, ITEM_BLOCK_SIZE) + position_offset->vz \
        )
        SVECTOR current_verts[4] = {
            createVert(v0),
            createVert(v1),
            createVert(v2),
            createVert(v3)
        };
        #undef createVert
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
                fixedMul(face_attribute->tint.r, light_level_colour_scalar),
                fixedMul(face_attribute->tint.g, light_level_colour_scalar),
                fixedMul(face_attribute->tint.b, light_level_colour_scalar)
            );
        } else {
           setRGB0(
                pol4,
                fixedMul(0xFF, light_level_colour_scalar),
                fixedMul(0xFF, light_level_colour_scalar),
                fixedMul(0xFF, light_level_colour_scalar)
            ); 
        }
        gte_ldrgb(&pol4->r0);
        // Load the face normal
        gte_ldv0(&FACE_DIRECTION_NORMALS[i]);
        // Apply RGB tinting to lighting calculation result on the basis
        // that it is enabled.
        // Normal Color Column Single
        gte_nccs();
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
        u32* ot_object = allocateOrderingTable(ctx, p);
        addPrim(ot_object, pol4);
        renderClearConstraintsIndex(ctx, p);
    }
}

void itemBlockRenderWorld(ItemBlock* item,
                          const Chunk* chunk,
                          RenderContext* ctx,
                          Transforms* transforms) {
    VECTOR position = vec3_const_rshift(
        item->item.world_physics_object->position,
        FIXED_POINT_SHIFT
    );
    // FIXME: The physics object position for items isn't
    //        properly aligned to the bounding box since
    //        the position (which should be the centre of
    //        the AABB) isn't aligned properly and thus when
    //        converting to world position and querying the
    //        light level, it can query the next block over
    //        (in the direction that the item moved when
    //        it was dropped) and thus can get a light level
    //        of 0 and the item is rendered as black in the
    //        world.
    const VECTOR world_position = vec3_const_div(
        position,
        BLOCK_SIZE
    );
    position.vy = -position.vy - ITEM_BLOCK_ANIM_LUT[item->item.bob_offset];
    // Include any distance animation for pickups
    position = vec3_add(
        position,
        item->item.position
    );
    // Calculate light level
    const LightLevel internal_light_level = worldGetInternalLightLevel(chunk->world);
    const LightLevel light_level = worldGetLightValue(chunk->world, &world_position);
    const u16 light_level_colour_scalar = lightLevelColourScalar(
        internal_light_level,
        light_level
    );
    DEBUG_LOG(
        "[ITEM] World pos: " VEC_PATTERN " Internal: %d Light: %d Scalar: %d\n",
        VEC_LAYOUT(world_position),
        internal_light_level,
        light_level,
        light_level_colour_scalar
    );
    renderCtxBindMatrix(
        ctx,
        transforms,
        &item->item.rotation,
        &position
    );
    if (item->item.stack_size <= 1) {
        renderItemBlock(
            item,
            light_level_colour_scalar,
            ctx,
            &item_stack_render_offsets[0]
        );
    } else if (item->item.stack_size <= 16) {
        #pragma GCC unroll 2
        for (int i = 0; i < 2; i++) {
            renderItemBlock(
                item,
                light_level_colour_scalar,
                ctx,
                &item_stack_render_offsets[i]
            );
        }
    } else if (item->item.stack_size <= 32) {
        #pragma GCC unroll 3
        for (int i = 0; i < 3; i++) {
            renderItemBlock(
                item,
                light_level_colour_scalar,
                ctx,
                &item_stack_render_offsets[i]
            );
        }
    } else if (item->item.stack_size <= 48) {
        #pragma GCC unroll 4
        for (int i = 0; i < 4; i++) {
            renderItemBlock(
                item,
                light_level_colour_scalar,
                ctx,
                &item_stack_render_offsets[i]
            );
        }
    } else {
        #pragma GCC unroll 5
        for (int i = 0; i < 5; i++) {
            renderItemBlock(
                item,
                light_level_colour_scalar,
                ctx,
                &item_stack_render_offsets[i]
            );
        }
    }
    item->item.rotation.vy = (item->item.rotation.vy + ITEM_ROTATION_QUANTA) % ONE;
    if (item->item.world_physics_object->flags.on_ground) {
        if (item->item.bob_offset <= 0) {
            item->item.bob_direction = 1;
        } else if (item->item.bob_offset >= ITEM_BLOCK_BOB_ANIM_SAMPLES) {
            item->item.bob_direction = -1;
        }
        item->item.bob_offset += item->item.bob_direction;
    }
    renderCtxUnbindMatrix();
}

void renderItemBlockInventory(ItemBlock* item,
                              RenderContext* ctx,
                              const VECTOR* screen_position,
                              const int size,
                              const u8* face_indices,
                              const u8 face_indices_count) {
    int p;
    TextureAttributes* face_attribute;
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    const i16 offset_screen_x = -CENTRE_X + screen_position->vx;
    const i16 offset_screen_y = -CENTRE_Y + screen_position->vy;
    int i;
    for (int idx = 0; idx < face_indices_count; idx++) {
        i = face_indices[idx];
        face_attribute = &item->face_attributes[i];
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        #define createVert(_v) vec3_i16( \
            convertToVertex(CUBE_INDICES[i]._v, 0, size), \
            convertToVertex(CUBE_INDICES[i]._v, 1, size), \
            convertToVertex(CUBE_INDICES[i]._v, 2, size) \
        )
        SVECTOR current_verts[4] = {
            createVert(v0),
            createVert(v1),
            createVert(v2),
            createVert(v3)
        };
        #undef createVert
        #undef convertToVertex
        gte_ldv3(
            &current_verts[0],
            &current_verts[1],
            &current_verts[2]
        );
        // Rotation, Translation and Perspective Triple
        gte_rtpt();
        gte_nclip();
        gte_stopz(&p);
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
        #define applyOffset(_idx) \
            pol4->x##_idx += offset_screen_x; \
            pol4->y##_idx += offset_screen_y
        applyOffset(0);
        applyOffset(1);
        applyOffset(2);
        applyOffset(3);
        #undef applyOffset
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
        u32* ot_object = allocateOrderingTable(ctx, 1);
        addPrim(ot_object, pol4);
        renderClearConstraintsIndex(ctx, 1);
    }
    if (item->item.stack_size < 2) {
        // Don't draw the amount of items if there is less than 2
        return;
    }
    const u8 stack_size = item->item.stack_size;
    // Faster print to avoid sprintf->vsprintf->vsnprintf
    char stack_count_text[3] = {
        stack_size < 10 ? ' ' : '0' + ((stack_size / 10) % 10),
        '0' + (stack_size % 10),
        '\0'
    };
    ctx->primitive = fontSort(
        allocateOrderingTable(ctx, 0),
        ctx->primitive,
        screen_position->vx - 7,
        screen_position->vy, // -1 Offset accounts for shadow on text
        true,
        stack_count_text
    );
    renderClearConstraints(ctx);
}

MATRIX inventory_item_block_lighting_direction = {
    /* X,  Y,  Z */
    .m = {
        {FIXED_1_2, -FIXED_1_2, -FIXED_1_2},
        {0, 0, 0},
        {0, 0, 0}
    }
};
void itemBlockRenderInventory(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {
    VECTOR position = vec3_i32(0, 0, item->item.position.vz);
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&item->item.rotation, &omtx);
    TransMatrix(&omtx, &position);
    // Multiply light matrix to object matrix
    MulMatrix0(&inventory_item_block_lighting_direction, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    gte_SetBackColor(128, 128, 128);
    gte_SetFarColor(1, 1, 1);
    // TODO: Remove extra SLOT_DELTA offset, leaving it to
    //       be added as necessary by caller of this function
    VECTOR screen_position = vec3_i32(
        item->item.position.vx + (SLOT_DELTA_X / 2),
        item->item.position.vy + (SLOT_DELTA_Y / 2),
        0
    );
    renderItemBlockInventory(
        item,
        ctx,
        &screen_position,
        ITEM_BLOCK_INVENTORY_SIZE,
        ISOMETRIC_BLOCK_FACE_INDICES,
        ISOMETRIC_BLOCK_FACE_INDICES_COUNT
    );
    PopMatrix();
    gte_SetBackColor(
        back_colour.r,
        back_colour.g,
        back_colour.b
    );
    gte_SetFarColor(
        far_colour.r,
        far_colour.g,
        far_colour.b
    );
}

void itemBlockRenderHand(ItemBlock* item, RenderContext* ctx, Transforms* transforms) {

}

void itemBlockApplyInventoryRenderAttributes(ItemBlock* item) {
    item->item.position = ITEM_BLOCK_INVENTORY_POSITION_RENDER_ATTRIBUTE;
    item->item.rotation = ITEM_BLOCK_INVENTORY_ROTATION_RENDER_ATTRIBUTE;
}
