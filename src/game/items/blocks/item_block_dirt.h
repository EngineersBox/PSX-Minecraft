#pragma once

#ifndef PSXMC_ITEM_BLOCK_DIRT_H
#define PSXMC_ITEM_BLOCK_DIRT_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(DirtItemBlock);

#define dirtItemBlockAttributes() declareItemAttributes("dirt")

void DirtItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderWorldV");
void DirtItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderInventoryV");
void DirtItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderHandV");

void DirtItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");
void DirtItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");
void DirtItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, DirtItemBlock);

void dirtItemBlockInit(VSelf);
void DirtItemBlock_init(VSelf);

void dirtItemBlockDestroy(VSelf);
void DirtItemBlock_destroy(VSelf);

impl(IItem, DirtItemBlock);

ALLOC_CALL(DirtItemBlock_destroy, 1) DirtItemBlock* dirtItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(dirt);

#endif // PSXMC_ITEM_BLOCK_DIRT_H
