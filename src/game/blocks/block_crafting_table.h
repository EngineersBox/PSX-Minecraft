#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_CRAFTING_TABLE_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_CRAFTING_TABLE_H_

#include <interface99.h>

#include "block.h"
#include "../../core/input/input.h"
#include "../../entity/player.h"

DEFN_BLOCK_STATELESS(CraftingTableBlock, CRAFTING_TABLE);

DEFN_BLOCK_CONSTRUCTOR(craftingTable);

#define craftingTableBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = BLOCK_DEFAULT_HARDNESS, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_AXE, \
    .tool_material = ITEMMATERIAL_NONE, \
    .can_harvest = toolTypeBitset(1,1,1,1,1,1), \
    .propagates_sunlight =  false , \
    .propagates_blocklight =  false , \
    .face_attributes = CRAFTING_TABLE_FACE_ATTRIBUTES, \
    .name = "crafting_table" \
})

#define craftingTableBlockFaceAttributes() P99_PROTECT({\
    declareFaceAttributes(4,43,59,59,60,60) \
})

#define CRAFTING_TABLE_TEXTURE_WIDTH 176
#define CRAFTING_TABLE_TEXTURE_HEIGHT 166

// Crafting slots
#define CRAFTING_TABLE_SLOT_GROUP_DIMENSIONS_X 3
#define CRAFTING_TABLE_SLOT_GROUP_DIMENSIONS_Y 3
#define CRAFTING_TABLE_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define CRAFTING_TABLE_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define CRAFTING_TABLE_SLOT_GROUP_SLOT_SPACING_X 2
#define CRAFTING_TABLE_SLOT_GROUP_SLOT_SPACING_Y 2
// TODO: determine these screen positions
#define CRAFTING_TABLE_SLOT_GROUP_ORIGIN_X 0
#define CRAFTING_TABLE_SLOT_GROUP_ORIGIN_Y 0
#define CRAFTING_TABLE_SLOT_GROUP_INDEX_OFFSET 0
slotGroupCheck(CRAFTING_TABLE);

// Crafting result slots
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_DIMENSIONS_X 1
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_DIMENSIONS_Y 1
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_SLOT_SPACING_X 0
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_SLOT_SPACING_Y 0
// TODO: determine these screen positions
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_ORIGIN_X 0
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_ORIGIN_Y 0
#define CRAFTING_TABLE_RESULT_SLOT_GROUP_INDEX_OFFSET 10
slotGroupCheck(CRAFTING_TABLE_RESULT);

extern Slot crafting_table_slots[(slotGroupSize(CRAFTING_TABLE) + slotGroupSize(CRAFTING_TABLE_RESULT))];
    
void craftingTableBlockInit(VSelf);
void CraftingTableBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* craftingTableBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* CraftingTableBlock_destroy(VSelf, bool drop_item);

ALLOC_CALL(itemDestroy, 1) IItem* craftingTableBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* CraftingTableBlock_provideItem(VSelf);

#define CraftingTableBlock_useAction_CUSTOM ()
bool craftingTableBlockUseAction(VSelf);
bool CraftingTableBlock_useAction(VSelf);

void craftingTableBlockRenderUI(RenderContext* ctx, Transforms* transforms);

impl(IBlock, CraftingTableBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_CRAFTING_TABLE_H_
