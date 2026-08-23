#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_FURNACE_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_FURNACE_H_

#include <interface99.h>

#include "block.h"
DEFN_BLOCK_STATEFUL(FurnaceBlock, FURNACE);

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


impl(IBlock, FurnaceBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_FURNACE_H_
