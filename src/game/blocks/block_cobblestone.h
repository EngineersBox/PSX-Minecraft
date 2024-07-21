#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_COBBLESTONE_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_COBBLESTONE_H_

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(CobblestoneBlock, COBBLESTONE);

DEFN_BLOCK_CONSTRUCTOR(cobblestone);

#define cobblestoneBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = BLOCK_DEFAULT_HARDNESS, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_PICKAXE, \
    .tool_material = ITEMMATERIAL_WOOD, \
    .can_harvest = toolTypeBitset(0,1,0,0,0,0), \
    .propagates_sunlight = false, \
    .propagates_blocklight = false, \
    .name = "cobblestone" \
})

void cobblestoneBlockInit(VSelf);
void CobblestoneBlock_init(VSelf);

void cobblestoneBlockAccess(VSelf);
void CobblestoneBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* cobblestoneBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* CobblestoneBlock_destroy(VSelf, bool drop_item);

void cobblestoneBlockUpdate(VSelf);
void CobblestoneBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* cobblestoneBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* CobblestoneBlock_provideItem(VSelf);

impl(IBlock, CobblestoneBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_COBBLESTONE_H_
