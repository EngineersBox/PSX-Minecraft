#pragma once

#ifndef PSX_MINECRAFT_BLOCK_STONE_H
#define PSX_MINECRAFT_BLOCK_STONE_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(STONE, StoneBlock);

IBlock* stoneBlockCreate();

// (0.16277136 / ((0.6 * 0.91) * (0.6 * 0.91) * (0.6 * 0.91))) * ONE = 4096
// ONE * 1.5 = 6144
// ONE * 10 = 40960
#define stoneBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = 4096, \
    .hardness = 6144, \
    .resistance = 40960, \
    .name = "stone" \
})

void stoneBlockInit(VSelf);
void StoneBlock_init(VSelf);

void stoneBlockAccess(VSelf);
void StoneBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* stoneBlockDestroy(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* StoneBlock_destroy(VSelf);

void stoneBlockUpdate(VSelf);
void StoneBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* stoneBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* StoneBlock_provideItem(VSelf);

impl(IBlock, StoneBlock);

#endif // PSX_MINECRAFT_BLOCK_STONE_H
