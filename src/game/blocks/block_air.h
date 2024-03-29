#pragma once

#ifndef PSX_MINECRAFT_BLOCK_AIR_H
#define PSX_MINECRAFT_BLOCK_AIR_H

#include <stdbool.h>
#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(AIR, AirBlock);

IBlock* airBlockCreate();

void airBlockInit(VSelf);
void AirBlock_init(VSelf);

void airBlockAccess(VSelf);
void AirBlock_access(VSelf);

IItem* airBlockDestroy(VSelf);
IItem* AirBlock_destroy(VSelf);

void airBlockUpdate(VSelf);
void AirBlock_update(VSelf);

bool airBlockIsOpaque(VSelf);
bool AirBlock_isOpaque(VSelf);

IItem* airBlockProvideItem(VSelf);
IItem* AirBlock_provideItem(VSelf);

#define AirBlock_isOpaque_CUSTOM ()
impl(IBlock, AirBlock);

#endif // PSX_MINECRAFT_BLOCK_AIR_H
