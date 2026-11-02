#pragma once

#ifndef PSXMC_ITEM_BLOCK_STONE_H
#define PSXMC_ITEM_BLOCK_STONE_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(StoneItemBlock);

#define stoneItemBlockAttributes() declareItemAttributes("stone")

void StoneItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderWorldV");
void StoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderInventoryV");
void StoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderHandV");

void StoneItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");
void StoneItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");
void StoneItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, StoneItemBlock);

void stoneItemBlockInit(VSelf);
void StoneItemBlock_init(VSelf);

void stoneItemBlockDestroy(VSelf);
void StoneItemBlock_destroy(VSelf);

impl(IItem, StoneItemBlock);

ALLOC_CALL(StoneItemBlock_destroy, 1) StoneItemBlock* stoneItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(stone);

#endif // PSXMC_ITEM_BLOCK_STONE_H
