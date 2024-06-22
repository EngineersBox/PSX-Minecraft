#include "breaking_state.h"

#include "../../math/fixed_point.h"
#include "../../math/math_utils.h"
#include "../../util/interface99_extensions.h"
#include "../items/items.h"
#include "blocks.h"

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
