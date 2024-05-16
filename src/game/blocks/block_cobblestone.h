#pragma once

#ifndef _PSX_MINECRAFT__GAME_BLOCKS__BLOCK_COBBLESTONE_H_
#define _PSX_MINECRAFT__GAME_BLOCKS__BLOCK_COBBLESTONE_H_

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(COBBLESTONE, CobblestoneBlock);

IBlock* cobblestoneBlockCreate();

#define cobblestoneBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = 2048, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .name = "cobblestone" \
})

void cobblestoneBlockInit(VSelf);
void CobblestoneBlock_init(VSelf);

void cobblestoneBlockAccess(VSelf);
void CobblestoneBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* cobblestoneBlockDestroy(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* CobblestoneBlock_destroy(VSelf);

void cobblestoneBlockUpdate(VSelf);
void CobblestoneBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* cobblestoneBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* CobblestoneBlock_provideItem(VSelf);

impl(IBlock, CobblestoneBlock);

#endif // _PSX_MINECRAFT__GAME_BLOCKS__BLOCK_COBBLESTONE_H_