#pragma once

#ifndef PSX_MINECRAFT_STONE_BLOCK_H
#define PSX_MINECRAFT_STONE_BLOCK_H

#include <interface99.h>

#include "block.h"

typedef struct {
    Block block; // Composition with parent
} StoneBlock;

void stoneBlockInit(VSelf);
void StoneBlock_init(VSelf);

void stoneBlockAccess(VSelf);
void StoneBlock_access(VSelf);

void stoneBlockDestroy(VSelf);
void StoneBlock_destroy(VSelf);

void stoneBlockUpdate(VSelf);
void StoneBlock_update(VSelf);

impl(IBlock, StoneBlock);

extern IBlock STONE_BLOCK_SINGLETON;

#endif // PSX_MINECRAFT_STONE_BLOCK_H
