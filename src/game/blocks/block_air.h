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

void airBlockDestroy(VSelf, IItem* item_result);
void AirBlock_destroy(VSelf, IItem* item_result);

void airBlockUpdate(VSelf);
void AirBlock_update(VSelf);

bool airBlockIsOpaque(VSelf);
bool AirBlock_isOpaque(VSelf);

void airBlockProvideItem(VSelf, IItem* item);
void AirBlock_provideItem(VSelf, IItem* item);

#define AirBlock_isOpaque_CUSTOM ()
impl(IBlock, AirBlock);

#endif // PSX_MINECRAFT_BLOCK_AIR_H
