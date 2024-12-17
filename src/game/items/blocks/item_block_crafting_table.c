#include "item_block_crafting_table.h"

#include <stdlib.h>
#include <interface99.h>

#include "../../../util/interface99_extensions.h"
#include "../../../util/preprocessor.h"
#include "../../../math/vector.h"
#include "../item_id.h"

CraftingTableItemBlock* craftingTableItemBlockCreate() {
    return (CraftingTableItemBlock*) malloc(sizeof(CraftingTableItemBlock));
}

DEFN_ITEM_CONSTRUCTOR(craftingTable) {
    IItem* item = itemCreate();
    CraftingTableItemBlock* crafting_table_item_block = craftingTableItemBlockCreate();
    DYN_PTR(item, CraftingTableItemBlock, IItem, crafting_table_item_block);
    VCALL(*item, init);
    return item;
}

void craftingTableItemBlockDestroy(VSelf) ALIAS("CraftingTableItemBlock_destroy");
void CraftingTableItemBlock_destroy(VSelf) {
    VSELF(CraftingTableItemBlock);
    free(self);
}

void craftingTableItemBlockRenderWorld(VSelf, const Chunk* chunk,  RenderContext* ctx, Transforms* transforms) ALIAS("CraftingTableItemBlock_renderWorld");
void CraftingTableItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    VSELF(CraftingTableItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void craftingTableItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CraftingTableItemBlock_renderInventory");
void CraftingTableItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CraftingTableItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void craftingTableItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CraftingTableItemBlock_renderHand");
void CraftingTableItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CraftingTableItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void craftingTableItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("CraftingTableItemBlock_applyWorldRenderAttributes");
void CraftingTableItemBlock_applyWorldRenderAttributes(VSelf) {
    UNIMPLEMENTED();
}

void craftingTableItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("CraftingTableItemBlock_applyInventoryRenderAttributes");
void CraftingTableItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(CraftingTableItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void craftingTableItemBlockApplyHandRenderAttributes(VSelf) ALIAS("CraftingTableItemBlock_applyHandRenderAttributes");
void CraftingTableItemBlock_applyHandRenderAttributes(VSelf) {
    UNIMPLEMENTED();
}

void craftingTableItemBlockInit(VSelf) ALIAS("CraftingTableItemBlock_init");
void CraftingTableItemBlock_init(VSelf) {
    VSELF(CraftingTableItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_CRAFTING_TABLE,
            0
        ),
        .face_attributes = {declareTintedFaceAttributes(
            4, NO_TINT,
            43, NO_TINT,
            59, NO_TINT,
            59, NO_TINT,
            60, NO_TINT,
            60, NO_TINT
        )}
    };
}

void craftingTableItemBlockApplyDamage(VSelf) ALIAS("CraftingTableItemBlock_applyDamage");
void CraftingTableItemBlock_applyDamage(VSelf) {
    UNIMPLEMENTED();
}

void craftingTableItemBlockUseAction(VSelf) ALIAS("CraftingTableItemBlock_useAction");
void CraftingTableItemBlock_useAction(VSelf) {
    UNIMPLEMENTED();
}

void craftingTableItemBlockAttackAction(VSelf) ALIAS("CraftingTableItemBlock_attackAction");
void CraftingTableItemBlock_attackAction(VSelf) {
    UNIMPLEMENTED();
}
