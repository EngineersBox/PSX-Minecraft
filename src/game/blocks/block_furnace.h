#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_FURNACE_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_FURNACE_H_

#include <interface99.h>

#include "block.h"
#include "../gui/slot.h"
#include "../recipe/recipe.h"

#define FURNACE_TEXTURE_WIDTH 176
#define FURNACE_TEXTURE_HEIGHT 166

#define FURNACE_FIRE_TEXTURE_WIDTH 16
#define FURNACE_FIRE_TEXTURE_HEIGHT 16
// Position in texture sprite relative to top left
#define FURNACE_FIRE_TEXTURE_SRC_X FURNACE_TEXTURE_WIDTH
#define FURNACE_FIRE_TEXTURE_SRC_Y 0
#define FURNACE_FIRE_TEXTURE_POS_X (CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1) + 79)
#define FURNACE_FIRE_TEXTURE_POS_Y (CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1) + 35)

#define FURNACE_ARROW_TEXTURE_WIDTH 24
#define FURNACE_ARROW_TEXTURE_HEIGHT 16
// Position in texture sprite relative to top left
#define FURNACE_ARROW_TEXTURE_SRC_X FURNACE_TEXTURE_WIDTH
#define FURNACE_ARROW_TEXTURE_SRC_Y FURNACE_FIRE_TEXTURE_HEIGHT
#define FURNACE_ARROW_TEXTURE_POS_X (CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1) + 56)
#define FURNACE_ARROW_TEXTURE_POS_Y (CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1) + 36)

// Input slot
#define FURNACE_INPUT_SLOT_GROUP_DIMENSIONS_X 1
#define FURNACE_INPUT_SLOT_GROUP_DIMENSIONS_Y 1
#define FURNACE_INPUT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define FURNACE_INPUT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define FURNACE_INPUT_SLOT_GROUP_SLOT_SPACING_X 0
#define FURNACE_INPUT_SLOT_GROUP_SLOT_SPACING_Y 0
#define FURNACE_INPUT_SLOT_GROUP_ORIGIN_X (CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1) + 56)
#define FURNACE_INPUT_SLOT_GROUP_ORIGIN_Y (CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1) + 17)
#define FURNACE_INPUT_SLOT_GROUP_INDEX_OFFSET 0
slotGroupCheck(FURNACE_INPUT);

// Fuel slot
#define FURNACE_FUEL_SLOT_GROUP_DIMENSIONS_X 1
#define FURNACE_FUEL_SLOT_GROUP_DIMENSIONS_Y 1
#define FURNACE_FUEL_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define FURNACE_FUEL_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define FURNACE_FUEL_SLOT_GROUP_SLOT_SPACING_X 0
#define FURNACE_FUEL_SLOT_GROUP_SLOT_SPACING_Y 0
#define FURNACE_FUEL_SLOT_GROUP_ORIGIN_X (CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1) + 56)
#define FURNACE_FUEL_SLOT_GROUP_ORIGIN_Y (CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1) + 53)
#define FURNACE_FUEL_SLOT_GROUP_INDEX_OFFSET 1
slotGroupCheck(FURNACE_FUEL);

// Output slot
#define FURNACE_OUTPUT_SLOT_GROUP_DIMENSIONS_X 1
#define FURNACE_OUTPUT_SLOT_GROUP_DIMENSIONS_Y 1
#define FURNACE_OUTPUT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define FURNACE_OUTPUT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define FURNACE_OUTPUT_SLOT_GROUP_SLOT_SPACING_X 0
#define FURNACE_OUTPUT_SLOT_GROUP_SLOT_SPACING_Y 0
#define FURNACE_OUTPUT_SLOT_GROUP_ORIGIN_X (CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1) + 116)
#define FURNACE_OUTPUT_SLOT_GROUP_ORIGIN_Y (CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1) + 35)
#define FURNACE_OUTPUT_SLOT_GROUP_INDEX_OFFSET 2 
slotGroupCheck(FURNACE_OUTPUT);

DEFN_BLOCK_STATEFUL(FurnaceBlock, FURNACE,
    u16 fuel_burn_ticks;
    u16 cook_ticks;
    bool recipe_changed: 1;
    bool process_recipe: 1;
    u16 _pad: 14;
    RecipeQueryResult recipe;
    Slot furnace_slots[
        slotGroupSize(FURNACE_INPUT)
        + slotGroupSize(FURNACE_FUEL)
        + slotGroupSize(FURNACE_OUTPUT)
    ];
);

DEFN_BLOCK_CONSTRUCTOR(furnace);

#define furnaceBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = BLOCK_DEFAULT_HARDNESS, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_PICKAXE, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(0,1,0,0,0,0), \
    .propagates_sunlight =  false , \
    .propagates_blocklight =  false , \
    .face_attributes = FURNACE_FACE_ATTRIBUTES, \
    .name = "furnace" \
})

#define furnaceBlockFaceAttributes() P99_PROTECT({\
    declareFaceAttributes(62,62,44,45,45,45), \
    declareFaceAttributes(62,62,45,44,45,45), \
    declareFaceAttributes(62,62,45,45,44,45), \
    declareFaceAttributes(62,62,45,45,45,44) \
})

void furnaceBlockInit(VSelf);
void FurnaceBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* furnaceBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* FurnaceBlock_destroy(VSelf, bool drop_item);

ALLOC_CALL(itemDestroy, 1) IItem* furnaceBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* FurnaceBlock_provideItem(VSelf);
void furnaceBlockUpdate(VSelf);
void FurnaceBlock_update(VSelf);

#define FurnaceBlock_useAction_CUSTOM ()
bool furnaceBlockUseAction(VSelf);
bool FurnaceBlock_useAction(VSelf);

void furnaceBlockRenderUI(RenderContext* ctx, Transforms* transforms);

impl(IBlock, FurnaceBlock);
#undef FurnaceBlock_useAction_CUSTOM

#endif // _PSXMC__GAME_BLOCKS__BLOCK_FURNACE_H_
