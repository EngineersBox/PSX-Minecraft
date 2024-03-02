#pragma once

#ifndef PSX_MINECRAFT_BLOCK_AIR_H
#define PSX_MINECRAFT_BLOCK_AIR_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK(AirBlock);

AirBlock* airBlockCreate();

void airBlockInit(VSelf);
void AirBlock_init(VSelf);

void airBlockAccess(VSelf);
void AirBlock_access(VSelf);

void airBlockDestroy(VSelf);
void AirBlock_destroy(VSelf);

void airBlockUpdate(VSelf);
void AirBlock_update(VSelf);

impl(IBlock, AirBlock);

extern IBlock AIR_BLOCK_SINGLETON;

#endif // PSX_MINECRAFT_BLOCK_AIR_H
