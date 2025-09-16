#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_PLANK_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_PLANK_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(PlankItemBlock);

#define plankItemBlockAttributes() ((ItemAttributes) { \
    .max_stack_size = 64, \
    .max_durability = 0, \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_NONE, \
    .armour_type = ARMOURTYPE_NONE, \
    .material = ITEMMATERIAL_NONE, \
    .name = "plank" \
})

void plankItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void PlankItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void plankItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void PlankItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void plankItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void PlankItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void plankItemBlockApplyWorldRenderAttributes(VSelf);
void PlankItemBlock_applyWorldRenderAttributes(VSelf);

void plankItemBlockApplyInventoryRenderAttributes(VSelf);
void PlankItemBlock_applyInventoryRenderAttributes(VSelf);

void plankItemBlockApplyHandRenderAttributes(VSelf);
void PlankItemBlock_applyHandRenderAttributes(VSelf);

impl(Renderable, PlankItemBlock);

void plankItemBlockInit(VSelf);
void PlankItemBlock_init(VSelf);

void plankItemBlockDestroy(VSelf);
void PlankItemBlock_destroy(VSelf);impl(IItem, PlankItemBlock);

ALLOC_CALL(PlankItemBlock_destroy, 1) PlankItemBlock* plankItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(plank);

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_PLANK_H_