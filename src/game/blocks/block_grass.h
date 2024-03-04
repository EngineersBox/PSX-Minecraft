#pragma once

#ifndef PSX_MINECRAFT_BLOCK_GRASS_H
#define PSX_MINECRAFT_BLOCK_GRASS_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(GRASS, GrassBlock);

IBlock* grassBlockCreate();

void grassBlockInit(VSelf);
void GrassBlock_init(VSelf);

void grassBlockAccess(VSelf);
void GrassBlock_access(VSelf);

void grassBlockDestroy(VSelf);
void GrassBlock_destroy(VSelf);

void grassBlockUpdate(VSelf);
void GrassBlock_update(VSelf);

impl(IBlock, GrassBlock);

#endif // PSX_MINECRAFT_BLOCK_GRASS_H
