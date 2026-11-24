#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_PLANKS_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_PLANKS_H_

#include <interface99.h>

#include "block.h"
DEFN_BLOCK_STATELESS(PlankBlock, PLANK);

DEFN_BLOCK_CONSTRUCTOR(plank);

#define plankBlockCreateAttributes() declareBlockAttributes(\
    .tool_type = TOOL_TYPE_AXE, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(1,1,1,1,1,1), \
    .face_attributes = PLANK_FACE_ATTRIBUTES, \
    .name = "plank" \
)

#define plankBlockFaceAttributes() P99_PROTECT({\
    defaultFaceAttributes(4) \
})

void plankBlockInit(VSelf);
void PlankBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* plankBlockDestroy(VSelf, bool drop_item, const VECTOR block_world_pos);
ALLOC_CALL(itemDestroy, 1) IItem* PlankBlock_destroy(VSelf, bool drop_item, const VECTOR block_world_pos);

ALLOC_CALL(itemDestroy, 1) IItem* plankBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* PlankBlock_provideItem(VSelf);

impl(IBlock, PlankBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_PLANKS_H_
