#pragma once

#ifndef PSX_MINECRAFT_BLOCK_STONE_H
#define PSX_MINECRAFT_BLOCK_STONE_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK(STONE, StoneBlock);

IBlock* stoneBlockCreate();

void stoneBlockInit(VSelf);
void StoneBlock_init(VSelf);

void stoneBlockAccess(VSelf);
void StoneBlock_access(VSelf);

void stoneBlockDestroy(VSelf);
void StoneBlock_destroy(VSelf);

void stoneBlockUpdate(VSelf);
void StoneBlock_update(VSelf);

impl(IBlock, StoneBlock);

#endif // PSX_MINECRAFT_BLOCK_STONE_H
