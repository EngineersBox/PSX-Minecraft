#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_PLANK_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_PLANK_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(PlankItemBlock);

#define plankItemBlockAttributes() declareItemAttributes("plank", \
    .burn_ticks = 300 \
)

void PlankItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderWorldV");
void PlankItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderInventoryV");
void PlankItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderHandV");

void PlankItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");
void PlankItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");
void PlankItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, PlankItemBlock);

void plankItemBlockInit(VSelf);
void PlankItemBlock_init(VSelf);

void plankItemBlockDestroy(VSelf);
void PlankItemBlock_destroy(VSelf);impl(IItem, PlankItemBlock);

ALLOC_CALL(PlankItemBlock_destroy, 1) PlankItemBlock* plankItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(plank);

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_PLANK_H_
