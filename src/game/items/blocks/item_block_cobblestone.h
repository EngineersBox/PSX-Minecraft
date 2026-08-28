#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEM_BLOCK_COBBLESTONE_H_
#define _PSXMC__GAME_ITEMS__ITEM_BLOCK_COBBLESTONE_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"

DEFN_ITEM_BLOCK(CobblestoneItemBlock);

#define cobblestoneItemBlockAttributes() declareItemAttributes("cobblestone", \
    .tool_type = TOOLTYPE_PICKAXE, \
    .material = ITEMMATERIAL_WOOD \
)

void cobblestoneItemBlockRenderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);
void CobblestoneItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms);

void cobblestoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void CobblestoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void cobblestoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void CobblestoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void CobblestoneItemBlock_applyWorldRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyWorldRenderAttributes");

void CobblestoneItemBlock_applyInventoryRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyInventoryRenderAttributes");

void CobblestoneItemBlock_applyHandRenderAttributes(VSelf) ASM_ALIAS("itemBlockApplyHandRenderAttributes");

impl(Renderable, CobblestoneItemBlock);

void cobblestoneItemBlockInit(VSelf);
void CobblestoneItemBlock_init(VSelf);

void cobblestoneItemBlockDestroy(VSelf);
void CobblestoneItemBlock_destroy(VSelf);

impl(IItem, CobblestoneItemBlock);

ALLOC_CALL(CobblestoneItemBlock_destroy, 1) CobblestoneItemBlock* cobblestoneItemBlockCreate();
DEFN_ITEM_CONSTRUCTOR(cobblestone);

#endif // _PSXMC__GAME_ITEMS__ITEM_BLOCK_COBBLESTONE_H_
