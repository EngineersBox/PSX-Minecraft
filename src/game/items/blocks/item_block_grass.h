#pragma once

#ifndef PSXMC_ITEM_BLOCK_GRASS_H
#define PSXMC_ITEM_BLOCK_GRASS_H

#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(GrassItemBlock);

#define grassItemBlockAttributes() ((ItemAttributes) { \
    .max_stack_size = 64, \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_SHOVEL, \
    .armour_type = ARMOURTYPE_NONE, \
    .material = ITEMMATERIAL_WOOD, \
    .has_durability = false, \
    .name = "grass" \
})

void grassItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void grassItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void grassItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void GrassItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

/*void grassItemBlockApplyWorldRenderAttributes(VSelf);*/
void GrassItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");

/*void grassItemBlockApplyInventoryRenderAttributes(VSelf);*/
void GrassItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");

/*void grassItemBlockApplyHandRenderAttributes(VSelf);*/
void GrassItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, GrassItemBlock);

void grassItemBlockInit(VSelf);
void GrassItemBlock_init(VSelf);

void grassItemBlockDestroy(VSelf);
void GrassItemBlock_destroy(VSelf);

void grassItemBlockApplyDamage(VSelf);
void GrassItemBlock_applyDamage(VSelf);

void grassItemBlockUseAction(VSelf);
void GrassItemBlock_useAction(VSelf);

void grassItemBlockAttackAction(VSelf);
void GrassItemBlock_attackAction(VSelf);

impl(IItem, GrassItemBlock);

ALLOC_CALL(GrassItemBlock_destroy, 1) GrassItemBlock* grassItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(grass);

#endif // PSXMC_ITEM_BLOCK_GRASS_H
