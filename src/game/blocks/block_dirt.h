#pragma once

#ifndef PSX_MINECRAFT_BLOCK_DIRT_H
#define PSX_MINECRAFT_BLOCK_DIRT_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK(DirtBlock);

DirtBlock* dirtBlockCreate();

void dirtBlockInit(VSelf);
void DirtBlock_init(VSelf);

void dirtBlockAccess(VSelf);
void DirtBlock_access(VSelf);

void dirtBlockDestroy(VSelf);
void DirtBlock_destroy(VSelf);

void dirtBlockUpdate(VSelf);
void DirtBlock_update(VSelf);

impl(IBlock, DirtBlock);

extern IBlock DIRT_BLOCK_SINGLETON;

#endif // PSX_MINECRAFT_BLOCK_DIRT_H
