#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_CRAFTING_TABLE_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_CRAFTING_TABLE_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(CraftingTableItemBlock);

#define craftingTableItemBlockAttributes() ((ItemAttributes) { \
    .max_stack_size = 64, \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_NONE, \
    .armour_type = ARMOURTYPE_NONE, \
    .material = ITEMMATERIAL_NONE, \
    .has_durability = false, \
    .name = "crafting_table" \
})

void craftingTableItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void CraftingTableItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void craftingTableItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void CraftingTableItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void craftingTableItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void CraftingTableItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void craftingTableItemBlockApplyWorldRenderAttributes(VSelf);
void CraftingTableItemBlock_applyWorldRenderAttributes(VSelf);

void craftingTableItemBlockApplyInventoryRenderAttributes(VSelf);
void CraftingTableItemBlock_applyInventoryRenderAttributes(VSelf);

void craftingTableItemBlockApplyHandRenderAttributes(VSelf);
void CraftingTableItemBlock_applyHandRenderAttributes(VSelf);

impl(Renderable, CraftingTableItemBlock);

void craftingTableItemBlockInit(VSelf);
void CraftingTableItemBlock_init(VSelf);

void craftingTableItemBlockDestroy(VSelf);
void CraftingTableItemBlock_destroy(VSelf);

void craftingTableItemBlockApplyDamage(VSelf);
void CraftingTableItemBlock_applyDamage(VSelf);

void craftingTableItemBlockUseAction(VSelf);
void CraftingTableItemBlock_useAction(VSelf);

void craftingTableItemBlockAttackAction(VSelf);
void CraftingTableItemBlock_attackAction(VSelf);

impl(IItem, CraftingTableItemBlock);

ALLOC_CALL(CraftingTableItemBlock_destroy, 1) CraftingTableItemBlock* craftingTableItemBlockCreate();

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_CRAFTING_TABLE_H_
