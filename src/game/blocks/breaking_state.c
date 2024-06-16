#include "breaking_state.h"

#include "../../math/fixed_point.h"
#include "../../util/interface99_extensions.h"
#include "blocks.h"
#include "../items/items.h"


void breakingStateCalculateTicks(BreakingState* state,
                                     const IItem* held_item,
                                     const bool in_water,
                                     const bool on_ground) {
    if (state->block == NULL) {
        return;
    }
    const Block* block = VCAST_PTR(Block*, state->block);
    const ToolType block_tool_type = blockGetToolType(block->id);
    const ToolMaterial block_tool_material = blockGetToolMaterial(block->id);
    ToolType item_tool_type = TOOLTYPE_NONE;
    ToolMaterial item_tool_material = TOOLMATERIAL_NONE;
    if (held_item != NULL) {
        const Item* item = VCAST_PTR(Item*, held_item);
        item_tool_type = itemGetToolType(item->id);
        item_tool_material = itemGetToolMaterial(item->id);
    }
    const bool is_ideal_tool_type = item_tool_type == block_tool_type;
    const bool tool_can_harvest_block = item_tool_material >= block_tool_material;
    fixedi32 speed_multiplier = 1;
    if (is_ideal_tool_type) {
        speed_multiplier = TOOL_MATERIAL_SPEED_MULTIPLIER[item_tool_material];
        if (!tool_can_harvest_block) {
            speed_multiplier = 1;
        }
    }
    if (in_water) {
        speed_multiplier /= 5;
    }
    if (!on_ground) {
        speed_multiplier /= 5;
    }
    fixedi32 damage = fixedDiv(speed_multiplier, (fixedi32) blockGetHardness(block->id));
    if (tool_can_harvest_block) {
        damage /= 30;
    } else {
        damage /= 100;
    }
    if (damage > ONE) {
        state->ticks_left = 0;
        return;
    }
    state->ticks_left = fixedDiv(ONE, damage); // In ticks
}
