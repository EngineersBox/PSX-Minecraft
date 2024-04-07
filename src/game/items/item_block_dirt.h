#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_DIRT_H
#define PSX_MINECRAFT_ITEM_BLOCK_DIRT_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"
#include "../../render/renderable.h"

DEFN_ITEM_BLOCK(DirtItemBlock);

void dirtItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms);
void DirtItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms);

void dirtItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void DirtItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void dirtItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void DirtItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void dirtItemBlockApplyWorldRenderAttributes(VSelf);
void DirtItemBlock_applyWorldRenderAttributes(VSelf);

void dirtItemBlockApplyInventoryRenderAttributes(VSelf);
void DirtItemBlock_applyInventoryRenderAttributes(VSelf);

void dirtItemBlockApplyHandRenderAttributes(VSelf);
void DirtItemBlock_applyHandRenderAttributes(VSelf);

impl(Renderable, DirtItemBlock);

void dirtItemBlockInit(VSelf);
void DirtItemBlock_init(VSelf);

void dirtItemBlockDestroy(VSelf);
void DirtItemBlock_destroy(VSelf);

void dirtItemBlockApplyDamage(VSelf);
void DirtItemBlock_applyDamage(VSelf);

void dirtItemBlockUseAction(VSelf);
void DirtItemBlock_useAction(VSelf);

void dirtItemBlockAttackAction(VSelf);
void DirtItemBlock_attackAction(VSelf);

impl(IItem, DirtItemBlock);

ALLOC_CALL(DirtItemBlock_destroy, 1) DirtItemBlock* dirtItemBlockCreate();

#endif // PSX_MINECRAFT_ITEM_BLOCK_DIRT_H
