#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_PLANKS_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_PLANKS_H_

#include <interface99.h>

#include "block.h"
DEFN_BLOCK_STATELESS(PlankBlock, PLANK);

DEFN_BLOCK_CONSTRUCTOR(plank);

#define plankBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = BLOCK_DEFAULT_HARDNESS, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_AXE, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(1,1,1,1,1,1), \
    .propagates_sunlight =  false , \
    .propagates_blocklight =  false , \
    .face_attributes = PLANK_FACE_ATTRIBUTES, \
    .name = "plank" \
})

#define plankBlockFaceAttributes() P99_PROTECT({\
defaultFaceAttributes(4) \
})

void plankBlockInit(VSelf);
void PlankBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* plankBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* PlankBlock_destroy(VSelf, bool drop_item);

ALLOC_CALL(itemDestroy, 1) IItem* plankBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* PlankBlock_provideItem(VSelf);

impl(IBlock, PlankBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_PLANKS_H_
