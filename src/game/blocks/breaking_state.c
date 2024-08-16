#include "breaking_state.h"

#include <psxgpu.h>
#include <psxgte.h>

#include "../../math/fixed_point.h"
#include "../../math/math_utils.h"
#include "../../util/interface99_extensions.h"
#include "../../structure/primitive/cube.h"
#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../render/commands.h"
#include "../items/items.h"
#include "blocks.h"

// Forward declaration
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);

const RECT breaking_texture_offscreen = (RECT) {
    .x = 832,
    .y = 0,
    .w = (16 * FACE_DIRECTION_COUNT),
    .h = 16
};

void breakingStateCalculateTicks(BreakingState* state,
                                 const IItem* held_item,
                                 const bool in_water,
                                 const bool on_ground) {
    if (state->block == NULL) {
        return;
    }
    const Block* block = VCAST_PTR(Block*, state->block);
    const ToolType block_tool_type = blockGetToolType(block->id);
    const ItemMaterial block_tool_material = blockGetToolMaterial(block->id);
    ToolType item_tool_type = TOOLTYPE_NONE;
    ItemMaterial item_tool_material = ITEMMATERIAL_NONE;
    if (held_item != NULL) {
        const Item* item = VCAST_PTR(Item*, held_item);
        const ItemID item_id = item->id;
        if (itemGetType(item_id) == ITEMTYPE_TOOL) {
            item_tool_type = itemGetToolType(item_id);
            item_tool_material = itemGetMaterial(item_id);
        }
    }
    const bool is_ideal_tool_type = item_tool_type == block_tool_type;
    const bool tool_can_harvest_block = blockCanHarvest(
        block_tool_type,
        block_tool_material,
        item_tool_type,
        item_tool_material,
        block
    );
    fixedi32 speed_multiplier = ONE;
    if (is_ideal_tool_type) {
        speed_multiplier = ITEM_MATERIAL_SPEED_MULTIPLIER[item_tool_material];
        if (!tool_can_harvest_block) {
            speed_multiplier = ONE;
        }
    }
    if (in_water) {
        speed_multiplier /= 5;
    }
    if (!on_ground) {
        speed_multiplier /= 5;
    }
    fixedi32 damage = fixedFixedDiv(speed_multiplier, (fixedi32) blockGetHardness(block->id));
    if (tool_can_harvest_block) {
        damage /= 30;
    } else {
        damage /= 100;
    }
    if (damage > ONE) {
        state->ticks_precise = 0;
        state->ticks_per_stage = 1;
        state->ticks_so_far = 0;
        return;
    }
    state->ticks_precise = fixedFixedDiv(ONE, damage);
    state->ticks_per_stage = ceilDiv(state->ticks_precise, 10); // 10 == number of breaking textures
    state->ticks_so_far = 0;
}

void breakingStateCalculateVisibility(BreakingState* state, const World* world) {
    state->visible_sides_bitset = 0;
    if (state->block == NULL) {
        return;
    }
    for (FaceDirection face_dir = 0; face_dir < FACE_DIRECTION_COUNT; face_dir++) {
        const VECTOR vis_check_position = vec3_i32(
            state->position.vx + CUBE_NORMS_UNIT[face_dir].vx,
            state->position.vy - CUBE_NORMS_UNIT[face_dir].vy, // Normal up direction is negative in world space and we use positive for arrays
            state->position.vz + CUBE_NORMS_UNIT[face_dir].vz
        );
        const IBlock* iblock = worldGetBlock(world, &vis_check_position);
        if (iblock == NULL) {
            goto set_visible;
        }
        const Block* facing_block = VCAST_PTR(Block*, iblock);
        if (facing_block->id != BLOCKID_AIR
            && blockIsFaceOpaque(facing_block, faceDirectionOpposing(face_dir))) {
            continue;
        }
set_visible:
        state->visible_sides_bitset |= (0b1 << face_dir);
    }
}

void breakingStateUpdateRenderTarget(BreakingState* state,
                                     RenderContext* ctx) {
    // 1. Check if we have progressed far enough in breaking to update the texture
    const fixedi32 ticks_per_stage = state->ticks_per_stage;
    const fixedi32 ticks_so_far = state->ticks_so_far;
    const bool next_texture = ticks_so_far == 0
        || (ticks_so_far / ticks_per_stage) > ((ticks_so_far - ONE) / ticks_per_stage);
    if (state->block == NULL || !next_texture) {
        state->chunk_remesh_trigger = false;
        return;
    }
    // 2. For each visible face, blit the face texture to the offset position in the render target
    // Sort drawing primitives to reset drawing area to default (OT is in reverse order so that's
    // why this is here and not after the loop).
    DR_AREA* area = (DR_AREA*) allocatePrimitive(ctx, sizeof(DR_AREA));
    const Framebuffer* active = &ctx->db[ctx->active];
    const Framebuffer* inactive = &ctx->db[1 - ctx->active];
    const size_t ot_index = ORDERING_TABLE_LENGTH - 1;
    setDrawArea(area, &inactive->draw_env.clip);
    const u32* ot_entry = &active->ordering_table[ot_index];
    addPrim(ot_entry, area);
    DR_OFFSET* offset = (DR_OFFSET*) allocatePrimitive(ctx, sizeof(DR_OFFSET));
    setDrawOffset(
        offset,
        inactive->draw_env.clip.x,
        inactive->draw_env.clip.y
    );
    addPrim(ot_entry, offset);
    // Reset texture window that will be enabled for applying the breaking
    // texture
    RECT tex_window = (RECT) {0, 0, 0, 0};
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_entry, ptwin);
    // Sort a textured polygon that covers the entire offscreem TPage
    // area and repeats the current stage of breaking texture over it
    // with 50% blending. This saves needing to do multiple SPRT prims
    // to cover the same area.
    const TextureAttributes face_attribute = (TextureAttributes) {
        .u = (BLOCK_TEXTURE_SIZE * 2) + ((state->ticks_so_far / state->ticks_per_stage) * BLOCK_TEXTURE_SIZE),
        .v = BLOCK_TEXTURE_SIZE * 14,
        .w = BLOCK_TEXTURE_SIZE * FACE_DIRECTION_COUNT,
        .h = BLOCK_TEXTURE_SIZE,
        .tint = {0, 0, 0, 0}
    };
    const Texture* terrain_texture = &textures[ASSET_TEXTURES_TERRAIN_INDEX];
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setSemiTrans(pol4, 1);
    // Note that coords are relative to the top left of the DR_AREA we set above
    setXYWH(
        pol4,
        0,
        0,
        BLOCK_TEXTURE_SIZE * FACE_DIRECTION_COUNT,
        BLOCK_TEXTURE_SIZE
    );
    setUVWH(
        pol4,
        face_attribute.u,
        face_attribute.v,
        face_attribute.w,
        face_attribute.h
    );
    setRGB0(pol4, 0x80, 0x80, 0x80);
    // TODO: Fix this blending to ensure that the overlay
    //       changes color according to the block texture
    setSemiTrans(pol4, true);
    pol4->tpage = terrain_texture->tpage;
    /*// Clear semi-transparency bits*/
    /*pol4->tpage &= ~(0b11 << 5);*/
    /*// Set custom semi-transparency*/
    /*pol4->tpage |= 0b11 << 5;*/
    pol4->clut = terrain_texture->clut;
    addPrim(ot_entry, pol4);
    // Texture window to ensure wrapping across offscreen TPage
    tex_window = (RECT) {
        // All in units of 8 pixels, hence right shift by 3
        .w = BLOCK_TEXTURE_SIZE >> 3,
        .h = BLOCK_TEXTURE_SIZE >> 3,
        .x = face_attribute.u >> 3,
        .y = face_attribute.v >> 3
    };
    ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_entry, ptwin);
    // 3. Sort sprite operations to render to offset positions and merge breaking texture with blending.
    //    This must be sorted before any other chunk rendering takes place to ensure that the texture is
    //    updated correctly in time
    const Block* block = VCAST_PTR(Block*, state->block);
    const TextureAttributes* attributes = blockGetFaceAttributes(block->id, block->metadata_id);
    for (FaceDirection face_dir = 0; face_dir < FACE_DIRECTION_COUNT; face_dir++) {
        if (((state->visible_sides_bitset >> face_dir) & 0b1) == 0) {
            continue;
        }
        pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setSemiTrans(pol4, true);
        const TextureAttributes* attribute = &attributes[face_dir];
        setXYWH(
            pol4,
            BLOCK_TEXTURE_SIZE * face_dir,
            0,
            BLOCK_TEXTURE_SIZE,
            BLOCK_TEXTURE_SIZE
        );
        setUVWH(
            pol4,
            attribute->u,
            attribute->v,
            BLOCK_TEXTURE_SIZE,
            BLOCK_TEXTURE_SIZE
        );
        // Commented out switch is used to render normals as distinct colours
        // switch (face_dir) {
        //     case FACE_DIR_DOWN: setRGB0(pol4,0xFF,0x00,0x00); break;
        //     case FACE_DIR_UP: setRGB0(pol4,0xFF,0xFF,0x00); break;
        //     case FACE_DIR_LEFT: setRGB0(pol4,0x00,0xFF,0x00); break;
        //     case FACE_DIR_RIGHT: setRGB0(pol4,0x00,0xFF,0xFF); break;
        //     case FACE_DIR_BACK: setRGB0(pol4,0x00,0x00,0xFF); break;
        //     case FACE_DIR_FRONT: setRGB0(pol4,0xFF,0x00,0xFF); break;
        // }
        setRGB0(
            pol4,
            0x80,
            0x80,
            0x80
        );
        pol4->tpage = terrain_texture->tpage;
        pol4->clut = terrain_texture->clut;
        addPrim(ot_entry, pol4);
    }
    // Reset any previous windows
    tex_window = (RECT) {0, 0, 0, 0};
    ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_entry, ptwin);
    // Sort drawing bindings to ensure we target offscreen TPage area
    area = (DR_AREA*) allocatePrimitive(ctx, sizeof(DR_AREA));
    setDrawArea(area, &breaking_texture_offscreen);
    addPrim(ot_entry, area);
    offset = (DR_OFFSET*) allocatePrimitive(ctx, sizeof(DR_OFFSET));
    setDrawOffset(
        offset,
        breaking_texture_offscreen.x,
        breaking_texture_offscreen.y
    );
    addPrim(ot_entry, offset);
    // 4. Trigger remeshing on chunk holding block
    state->chunk_remesh_trigger = true;
}
