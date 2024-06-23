#include "breaking_state.h"

#include <psxgpu.h>
#include <psxgte.h>

#include "../../math/fixed_point.h"
#include "../../math/math_utils.h"
#include "../../util/interface99_extensions.h"
#include "../items/items.h"
#include "blocks.h"

const RECT breaking_texture_offscreen = (RECT) {
    .x = 784 >> 3,
    .y = 0 >> 3,
    .w = (16 * FACE_DIRECTION_COUNT) >> 3,
    .h = 16 >> 3
};

bool canHarvestBlock(const ToolType block_tool_type,
                     const ItemMaterial block_tool_material,
                     const ToolType item_tool_type,
                     const ItemMaterial item_tool_material,
                     const Block* block) {
    const bool can_harvest = blockGetItemCanHarvest(block->id, item_tool_type);
    if (!can_harvest) {
        return false;
    }
    if (item_tool_type == TOOLTYPE_NONE) {
        return true;
    }
    return block_tool_type == item_tool_type && item_tool_material >= block_tool_material;
}

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
    const bool tool_can_harvest_block = canHarvestBlock(
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
    state->ticks_per_stage = ceilDiv(state->ticks_precise, 10);
    state->ticks_so_far = 0;
}

void breakingStateUpdateRenderTarget(const BreakingState* state, RenderContext* ctx) {
    // 1. Check if we have progressed far enough in breaking to update the texture
    const fixedi32 ticks_per_stage = state->ticks_per_stage;
    const fixedi32 ticks_so_far = state->ticks_so_far;
    const bool next_texture = ticks_so_far == 0 || (ticks_so_far / ticks_per_stage) > ((ticks_so_far - ONE) / ticks_per_stage);
    if (!next_texture) {
        return;
    }
    // 2. Determine which faces are visible on the target block
    const u8 bitset = VCALL(*state->block, opaqueBitset);
    // 3. For each visible face, blit the face texture to the offset position in the render target
    const Block* block = VCAST_PTR(Block*, state->block);
    const TextureAttributes* attributes = block->face_attributes;
    for (FaceDirection face_dir = 0; face_dir < FACE_DIRECTION_COUNT; face_dir++) {
        if (!((bitset >> face_dir) & 0b1)) {
            // Not set, not visible
            continue;
        }
        const TextureAttributes* attribute = &attributes[face_dir];
        const RECT reference_texture = (RECT) {
            .x = attribute->u,
            .y = attribute->v,
            .w = attribute->w,
            .h = attribute->h
        };
        MoveImage(
            &reference_texture,
            breaking_texture_offscreen.x + (face_dir * 16) >> 3,
            breaking_texture_offscreen.y
        );
    }
    // Wait for blitting to finish via GPU IRQ
    DrawSync(1);
    // 4. Sort sprite operations to render to offset positions and merge breaking texture with 50% blending.
    //    This must be sorted before any other chunk rendering takes place to ensure that the texture is

    //    updated correctly in time
    // 5. Trigger remeshing on chunk holding block
    // 6. (In meshing) if the block matches the breaking state ensure it has unique mesh primitives
    //    with correct tpage and texture window into the render target
}