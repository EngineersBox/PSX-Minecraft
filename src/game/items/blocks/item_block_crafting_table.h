#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_CRAFTING_TABLE_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_CRAFTING_TABLE_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(CraftingTableItemBlock);

#define craftingTableItemBlockAttributes() ((ItemAttributes) { \
    .max_stack_size = 64, \
    .max_durability = 0, \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_NONE, \
    .armour_type = ARMOURTYPE_NONE, \
    .material = ITEMMATERIAL_NONE, \
    .name = "crafting_table" \
})

void craftingTableItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void CraftingTableItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void craftingTableItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void CraftingTableItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void craftingTableItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void CraftingTableItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void CraftingTableItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");

void CraftingTableItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");

void CraftingTableItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, CraftingTableItemBlock);

void craftingTableItemBlockInit(VSelf);
void CraftingTableItemBlock_init(VSelf);

void craftingTableItemBlockDestroy(VSelf);
void CraftingTableItemBlock_destroy(VSelf);

impl(IItem, CraftingTableItemBlock);

ALLOC_CALL(CraftingTableItemBlock_destroy, 1) CraftingTableItemBlock* craftingTableItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(craftingTable);

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_CRAFTING_TABLE_H_
