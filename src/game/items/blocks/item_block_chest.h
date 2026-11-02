#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_CHEST_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_CHEST_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(ChestItemBlock);

#define chestItemBlockAttributes() ((ItemAttributes) { \
    .max_stack_size = 64, \
    .max_durability = 0, \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_NONE, \
    .armour_type = ARMOURTYPE_NONE, \
    .material = ITEMMATERIAL_NONE, \
    .name = "chest" \
})

void ChestItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderWorldV");
void ChestItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderInventoryV");
void ChestItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) ASM_ALIAS("itemBlockRenderHandV");

void ChestItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");
void ChestItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");
void ChestItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, ChestItemBlock);

void chestItemBlockInit(VSelf);
void ChestItemBlock_init(VSelf);

void chestItemBlockDestroy(VSelf);
void ChestItemBlock_destroy(VSelf);impl(IItem, ChestItemBlock);

ALLOC_CALL(ChestItemBlock_destroy, 1) ChestItemBlock* chestItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(chest);

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_CHEST_H_
