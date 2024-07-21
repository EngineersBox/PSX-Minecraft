#pragma once

#ifndef PSXMC_BLOCK_STONE_H
#define PSXMC_BLOCK_STONE_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(StoneBlock, STONE);

DEFN_BLOCK_CONSTRUCTOR(stone);

// ONE * 1.5 = 6144
// ONE * 10 = 40960
#define stoneBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = 6144, \
    .resistance = 40960, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_PICKAXE, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(0,1,0,0,0,0), \
    .propagates_sunlight = false, \
    .propagates_blocklight = false, \
    .name = "stone" \
})

void stoneBlockInit(VSelf);
void StoneBlock_init(VSelf);

void stoneBlockAccess(VSelf);
void StoneBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* stoneBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* StoneBlock_destroy(VSelf, bool drop_item);

void stoneBlockUpdate(VSelf);
void StoneBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* stoneBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* StoneBlock_provideItem(VSelf);

impl(IBlock, StoneBlock);

#endif // PSXMC_BLOCK_STONE_H
