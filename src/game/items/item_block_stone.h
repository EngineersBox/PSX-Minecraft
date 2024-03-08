#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_STONE_H
#define PSX_MINECRAFT_ITEM_BLOCK_STONE_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(StoneItemBlock);

StoneItemBlock* stoneItemBlockCreate();
void stoneItemBlockDestroy(StoneItemBlock* stone_item_block);

void stoneItemBlockInit(VSelf);
void StoneItemBlock_init(VSelf);

void stoneItemBlockApplyDamage(VSelf);
void StoneItemBlock_applyDamage(VSelf);

void stoneItemBlockUseAction(VSelf);
void StoneItemBlock_useAction(VSelf);

void stoneItemBlockAttackAction(VSelf);
void StoneItemBlock_attackAction(VSelf);

impl(IItem, StoneItemBlock);

#endif // PSX_MINECRAFT_ITEM_BLOCK_STONE_H
