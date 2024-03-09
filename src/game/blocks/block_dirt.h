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

void dirtBlockDestroy(VSelf, IItem* item_result);
void DirtBlock_destroy(VSelf, IItem* item_result);

void dirtBlockUpdate(VSelf);
void DirtBlock_update(VSelf);

void dirtBlockProvideItem(VSelf, IItem* item);
void DirtBlock_provideItem(VSelf, IItem* item);

impl(IBlock, DirtBlock);


#endif // PSX_MINECRAFT_BLOCK_DIRT_H
