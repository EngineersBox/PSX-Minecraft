#pragma once

#ifndef PSX_MINECRAFT_BLOCK_STONE_H
#define PSX_MINECRAFT_BLOCK_STONE_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(STONE, StoneBlock);

IBlock* stoneBlockCreate();

void stoneBlockInit(VSelf);
void StoneBlock_init(VSelf);

void stoneBlockAccess(VSelf);
void StoneBlock_access(VSelf);

void stoneBlockDestroy(VSelf, IItem* item_result);
void StoneBlock_destroy(VSelf, IItem* item_result);

void stoneBlockUpdate(VSelf);
void StoneBlock_update(VSelf);

void stoneBlockProvideItem(VSelf, IItem* item);
void StoneBlock_provideItem(VSelf, IItem* item);

impl(IBlock, StoneBlock);

#endif // PSX_MINECRAFT_BLOCK_STONE_H
