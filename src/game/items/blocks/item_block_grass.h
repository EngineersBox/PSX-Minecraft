#pragma once

#ifndef PSXMC_ITEM_BLOCK_GRASS_H
#define PSXMC_ITEM_BLOCK_GRASS_H

#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(GrassItemBlock);

#define grassItemBlockAttributes() declareItemAttributes("grass")

void grassItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void grassItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void grassItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void GrassItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");

void GrassItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");

void GrassItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, GrassItemBlock);

void grassItemBlockInit(VSelf);
void GrassItemBlock_init(VSelf);

void grassItemBlockDestroy(VSelf);
void GrassItemBlock_destroy(VSelf);

impl(IItem, GrassItemBlock);

ALLOC_CALL(GrassItemBlock_destroy, 1) GrassItemBlock* grassItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(grass);

#endif // PSXMC_ITEM_BLOCK_GRASS_H
