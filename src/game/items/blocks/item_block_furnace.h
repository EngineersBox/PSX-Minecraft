#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_FURNACE_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_FURNACE_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(FurnaceItemBlock);

#define furnaceItemBlockAttributes() declareItemAttributes("furnace")

void FurnaceItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderWorldV");
void FurnaceItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderInventoryV");
void FurnaceItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderHandV");

void FurnaceItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");
void FurnaceItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");
void FurnaceItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, FurnaceItemBlock);

void furnaceItemBlockInit(VSelf);
void FurnaceItemBlock_init(VSelf);

void furnaceItemBlockDestroy(VSelf);
void FurnaceItemBlock_destroy(VSelf);impl(IItem, FurnaceItemBlock);

ALLOC_CALL(FurnaceItemBlock_destroy, 1) FurnaceItemBlock* furnaceItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(furnace);

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_FURNACE_H_
