#pragma once

#ifndef PSXMC_BLOCK_GRASS_H
#define PSXMC_BLOCK_GRASS_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(GrassBlock, GRASS);

DEFN_BLOCK_CONSTRUCTOR(grass);

// ONE * 0.6 = 2457
#define grassBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = 2457, \
    .resistance = 12288, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_SHOVEL, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(1,1,1,1,1,1), \
    .name = "grass" \
})

void grassBlockInit(VSelf);
void GrassBlock_init(VSelf);

void grassBlockAccess(VSelf);
void GrassBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* grassBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* GrassBlock_destroy(VSelf, bool drop_item);

void grassBlockUpdate(VSelf);
void GrassBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* grassBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* GrassBlock_provideItem(VSelf);

impl(IBlock, GrassBlock);

#endif // PSXMC_BLOCK_GRASS_H
