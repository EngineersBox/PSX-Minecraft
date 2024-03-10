#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_STONE_H
#define PSX_MINECRAFT_ITEM_BLOCK_STONE_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(StoneItemBlock);

StoneItemBlock* stoneItemBlockCreate();
void stoneItemBlockDestroy(StoneItemBlock* stone_item_block);

void stoneItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms);
void StoneItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms);

void stoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void StoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void stoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void StoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

impl(Renderable, StoneItemBlock);

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
