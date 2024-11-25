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
    .face_attributes CRAFTING_TABLE_FACE_ATTRIBUTES, \
    .name = "crafting_table" \
})

#define craftingTableBlockFaceAttributes() P99_PROTECT({\
    defaultTintedFaceAttributes(4,43,59,59,60,60) \
})

void craftingTableBlockInit(VSelf);
void CraftingTableBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* craftingTableBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* CraftingTableBlock_destroy(VSelf, bool drop_item);

ALLOC_CALL(itemDestroy, 1) IItem* craftingTableBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* CraftingTableBlock_provideItem(VSelf);

#define CraftingTableBlock_useAction_CUSTOM ()
bool craftingTableBlockUseAction(VSelf);
bool CraftingTableBlock_useAction(VSelf);

impl(IBlock, CraftingTableBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_CRAFTING_TABLE_H_
