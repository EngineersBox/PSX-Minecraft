#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_STONE_H
#define PSX_MINECRAFT_ITEM_BLOCK_STONE_H

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(StoneItemBlock);

#define stoneItemBlockAttributes() ((ItemAttributes) { \
    .type = ITEMTYPE_BLOCK, \
    .tool_type = TOOLTYPE_PICKAXE, \
    .tool_material = TOOLMATERIAL_WOOD, \
    .has_durability = false, \
    .name = "stone" \
})

void stoneItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms);
void StoneItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms);

void stoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void StoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void stoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void StoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void stoneItemBlockApplyWorldRenderAttributes(VSelf);
void StoneItemBlock_applyWorldRenderAttributes(VSelf);

void stoneItemBlockApplyInventoryRenderAttributes(VSelf);
void StoneItemBlock_applyInventoryRenderAttributes(VSelf);

void stoneItemBlockApplyHandRenderAttributes(VSelf);
void StoneItemBlock_applyHandRenderAttributes(VSelf);

impl(Renderable, StoneItemBlock);

void stoneItemBlockInit(VSelf);
void StoneItemBlock_init(VSelf);

void stoneItemBlockDestroy(VSelf);
void StoneItemBlock_destroy(VSelf);

void stoneItemBlockApplyDamage(VSelf);
void StoneItemBlock_applyDamage(VSelf);

void stoneItemBlockUseAction(VSelf);
void StoneItemBlock_useAction(VSelf);

void stoneItemBlockAttackAction(VSelf);
void StoneItemBlock_attackAction(VSelf);

impl(IItem, StoneItemBlock);

ALLOC_CALL(StoneItemBlock_destroy, 1) StoneItemBlock* stoneItemBlockCreate();

#endif // PSX_MINECRAFT_ITEM_BLOCK_STONE_H
