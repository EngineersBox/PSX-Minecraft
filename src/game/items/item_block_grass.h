#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_GRASS_H
#define PSX_MINECRAFT_ITEM_BLOCK_GRASS_H

#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(GrassItemBlock);

GrassItemBlock* grassItemBlockCreate();

void grassItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms);

void grassItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void grassItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void grassItemBlockApplyWorldRenderAttributes(VSelf);
void GrassItemBlock_applyWorldRenderAttributes(VSelf);

void grassItemBlockApplyInventoryRenderAttributes(VSelf);
void GrassItemBlock_applyInventoryRenderAttributes(VSelf);

void grassItemBlockApplyHandRenderAttributes(VSelf);
void GrassItemBlock_applyHandRenderAttributes(VSelf);

impl(Renderable, GrassItemBlock);

void grassItemBlockInit(VSelf);
void GrassItemBlock_init(VSelf);

void grassItemBlockDestroy(VSelf);
void GrassItemBlock_destroy(VSelf);

void grassItemBlockApplyDamage(VSelf);
void GrassItemBlock_applyDamage(VSelf);

void grassItemBlockUseAction(VSelf);
void GrassItemBlock_useAction(VSelf);

void grassItemBlockAttackAction(VSelf);
void GrassItemBlock_attackAction(VSelf);

impl(IItem, GrassItemBlock);

#endif // PSX_MINECRAFT_ITEM_BLOCK_GRASS_H
