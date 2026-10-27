#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_CHEST_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_CHEST_H_

#include <interface99.h>

#include "block.h"
#include "../gui/slot.h"

// Single chest slots
#define CHEST_SINGLE_SLOT_GROUP_DIMENSIONS_X 9
#define CHEST_SINGLE_SLOT_GROUP_DIMENSIONS_Y 3
#define CHEST_SINGLE_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define CHEST_SINGLE_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define CHEST_SINGLE_SLOT_GROUP_SLOT_SPACING_X 2
#define CHEST_SINGLE_SLOT_GROUP_SLOT_SPACING_Y 2
// TODO: Set these
#define CHEST_SINGLE_SLOT_GROUP_ORIGIN_X 0
#define CHEST_SINGLE_SLOT_GROUP_ORIGIN_Y 0
#define CHEST_SINGLE_SLOT_GROUP_INDEX_OFFSET 0
slotGroupCheck(CHEST_SINGLE);

// Double chest left slots (top of GUI)
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_DIMENSIONS_X 9
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_DIMENSIONS_Y 3
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_SLOT_SPACING_X 2
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_SLOT_SPACING_Y 2
// TODO: Set these
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_ORIGIN_X 0
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_ORIGIN_Y 0
#define CHEST_DOUBLE_LEFT_SLOT_GROUP_INDEX_OFFSET 0
slotGroupCheck(CHEST_DOUBLE_LEFT);

// Double chest left slots (bottom of GUI)
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_DIMENSIONS_X 9
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_DIMENSIONS_Y 3
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_SLOT_SPACING_X 2
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_SLOT_SPACING_Y 2
// TODO: Set these
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_ORIGIN_X 0
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_ORIGIN_Y 0
#define CHEST_DOUBLE_RIGHT_SLOT_GROUP_INDEX_OFFSET slotGroupSize(CHEST_DOUBLE_LEFT)
slotGroupCheck(CHEST_DOUBLE_RIGHT);

// NOTE: Using the chest type and the block orientation,
//       we can figure out where the other half of a
//       double chest is, avoiding the need to maintain
//       pointers to each chest in their counterparts
typedef enum ChestType {
    CHEST_TYPE_SINGLE = 0,
    CHEST_TYPE_DOUBLE_LEFT = 1,
    CHEST_TYPE_DOUBLE_RIGHT = 2
} ChestType;

DEFN_BLOCK_STATEFUL(ChestBlock, CHEST,
    ChestType type;
    u8 _pad;
    Slot slots[slotGroupSize(CHEST_SINGLE)];
);

DEFN_BLOCK_CONSTRUCTOR(chest);

#define chestBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = BLOCK_DEFAULT_HARDNESS, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_AXE, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(0,0,1,0,0,0), \
    .propagates_sunlight = false, \
    .propagates_blocklight = false, \
    .face_attributes = CHEST_FACE_ATTRIBUTES, \
    .name = "chest" \
})

#define chestBlockFaceAttributes() P99_PROTECT({\
    declareFaceAttributes(26,26,28,27,27,27), /* Single left */\
    declareFaceAttributes(26,26,27,28,27,27), /* Single right */\
    declareFaceAttributes(26,26,27,27,28,27), /* Single back */\
    declareFaceAttributes(26,26,27,27,27,28), /* Single front */\
    declareFaceAttributes(26,26,41,58,27,27), /* Double left left */\
    declareFaceAttributes(26,26,58,41,27,27), /* Double left right */\
    declareFaceAttributes(26,26,27,27,41,58), /* Double left back */\
    declareFaceAttributes(26,26,27,27,58,41), /* Double left front */\
    declareFaceAttributes(26,26,42,57,27,27), /* Double right left */\
    declareFaceAttributes(26,26,57,42,27,27), /* Double right right */\
    declareFaceAttributes(26,26,27,27,42,57), /* Double right back */\
    declareFaceAttributes(26,26,27,27,57,42) /* Double right front */\
})

void chestBlockInit(VSelf);
void ChestBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* chestBlockDestroy(VSelf, bool drop_item, const VECTOR block_world_pos);
ALLOC_CALL(itemDestroy, 1) IItem* ChestBlock_destroy(VSelf, bool drop_item, const VECTOR block_world_pos);

ALLOC_CALL(itemDestroy, 1) IItem* chestBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* ChestBlock_provideItem(VSelf);
BlockUpdateResultBitmap chestBlockUpdate(VSelf, const VECTOR block_world_pos);
BlockUpdateResultBitmap ChestBlock_update(VSelf, const VECTOR block_world_pos);

#define ChestBlock_useAction_CUSTOM ()
bool chestBlockUseAction(VSelf);
bool ChestBlock_useAction(VSelf);

impl(IBlock, ChestBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_CHEST_H_
