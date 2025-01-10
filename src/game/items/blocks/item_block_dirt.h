#pragma once

#ifndef PSXMC_ITEM_BLOCK_DIRT_H
#define PSXMC_ITEM_BLOCK_DIRT_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(DirtItemBlock);

#define dirtItemBlockAttributes() ((ItemAttributes) { \
    .max_stack_size = 64, \
    .max_durability = 0, \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_SHOVEL, \
    .armour_type = ARMOURTYPE_NONE, \
    .material = ITEMMATERIAL_WOOD, \
    .name = "dirt" \
})

void dirtItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void DirtItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void dirtItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void DirtItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void dirtItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void DirtItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

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
