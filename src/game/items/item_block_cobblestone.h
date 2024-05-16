#pragma once

#ifndef _PSX_MINECRAFT__GAME_ITEMS__ITEM_BLOCK_COBBLESTONE_H_
#define _PSX_MINECRAFT__GAME_ITEMS__ITEM_BLOCK_COBBLESTONE_H_

#include <stddef.h>
#include <interface99.h>

#include "item_block.h"
#include "../../render/renderable.h"

DEFN_ITEM_BLOCK(CobblestoneItemBlock);

void cobblestoneItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms);
void CobblestoneItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms);

void cobblestoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms);
void CobblestoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms);

void cobblestoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms);
void CobblestoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms);

void cobblestoneItemBlockApplyWorldRenderAttributes(VSelf);
void CobblestoneItemBlock_applyWorldRenderAttributes(VSelf);

void cobblestoneItemBlockApplyInventoryRenderAttributes(VSelf);
void CobblestoneItemBlock_applyInventoryRenderAttributes(VSelf);

void cobblestoneItemBlockApplyHandRenderAttributes(VSelf);
void CobblestoneItemBlock_applyHandRenderAttributes(VSelf);

impl(Renderable, CobblestoneItemBlock);

void cobblestoneItemBlockInit(VSelf);
void CobblestoneItemBlock_init(VSelf);

void cobblestoneItemBlockDestroy(VSelf);
void CobblestoneItemBlock_destroy(VSelf);

void cobblestoneItemBlockApplyDamage(VSelf);
void CobblestoneItemBlock_applyDamage(VSelf);

void cobblestoneItemBlockUseAction(VSelf);
void CobblestoneItemBlock_useAction(VSelf);

void cobblestoneItemBlockAttackAction(VSelf);
void CobblestoneItemBlock_attackAction(VSelf);

impl(IItem, CobblestoneItemBlock);

ALLOC_CALL(CobblestoneItemBlock_destroy, 1) CobblestoneItemBlock* cobblestoneItemBlockCreate();

#endif // _PSX_MINECRAFT__GAME_ITEMS__ITEM_BLOCK_COBBLESTONE_H_