#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_DIRT_H
#define PSX_MINECRAFT_ITEM_BLOCK_DIRT_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(DirtItemBlock);

DirtItemBlock* dirtItemBlockCreate();
void dirtItemBlockDestroy(DirtItemBlock* dirt_item_block);

void dirtItemBlockInit(VSelf);
void DirtItemBlock_init(VSelf);

void dirtItemBlockApplyDamage(VSelf);
void DirtItemBlock_applyDamage(VSelf);

void dirtItemBlockUseAction(VSelf);
void DirtItemBlock_useAction(VSelf);

void dirtItemBlockAttackAction(VSelf);
void DirtItemBlock_attackAction(VSelf);

impl(IItem, DirtItemBlock);

#endif // PSX_MINECRAFT_ITEM_BLOCK_DIRT_H
