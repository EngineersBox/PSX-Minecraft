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

IItem* dirtBlockDestroy(VSelf);
IItem* DirtBlock_destroy(VSelf);

void dirtBlockUpdate(VSelf);
void DirtBlock_update(VSelf);

IItem* dirtBlockProvideItem(VSelf);
IItem* DirtBlock_provideItem(VSelf);

impl(IBlock, DirtBlock);


#endif // PSX_MINECRAFT_BLOCK_DIRT_H
