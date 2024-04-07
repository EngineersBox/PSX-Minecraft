#pragma once

#ifndef PSX_MINECRAFT_BLOCK_DIRT_H
#define PSX_MINECRAFT_BLOCK_DIRT_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(DIRT, DirtBlock);

IBlock* dirtBlockCreate();

void dirtBlockInit(VSelf);
void DirtBlock_init(VSelf);

void dirtBlockAccess(VSelf);
void DirtBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* dirtBlockDestroy(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* DirtBlock_destroy(VSelf);

void dirtBlockUpdate(VSelf);
void DirtBlock_update(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* dirtBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* DirtBlock_provideItem(VSelf);

impl(IBlock, DirtBlock);


#endif // PSX_MINECRAFT_BLOCK_DIRT_H
