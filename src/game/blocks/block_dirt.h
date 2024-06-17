#pragma once

#ifndef PSX_MINECRAFT_BLOCK_DIRT_H
#define PSX_MINECRAFT_BLOCK_DIRT_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(DIRT, DirtBlock);

IBlock* dirtBlockCreate();

// (0.16277136 / ((0.6 * 0.91) * (0.6 * 0.91) * (0.6 * 0.91))) * ONE = 4096
// ONE * 0.5 = 2048
#define dirtBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = 2048, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_SHOVEL, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .name = "dirt" \
})

void dirtBlockInit(VSelf);
void DirtBlock_init(VSelf);

void dirtBlockAccess(VSelf);
void DirtBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* dirtBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* DirtBlock_destroy(VSelf, bool drop_item);

void dirtBlockUpdate(VSelf);
void DirtBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* dirtBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* DirtBlock_provideItem(VSelf);

impl(IBlock, DirtBlock);


#endif // PSX_MINECRAFT_BLOCK_DIRT_H
