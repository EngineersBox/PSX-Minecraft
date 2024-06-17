#include "item_block_dirt.h"

#include <stdlib.h>

#include "item_id.h"

DirtItemBlock* dirtItemBlockCreate() {
    return (DirtItemBlock*) malloc(sizeof(DirtItemBlock));
}

void dirtItemBlockDestroy(VSelf) ALIAS("DirtItemBlock_destroy");
void DirtItemBlock_destroy(VSelf) {
    VSELF(DirtItemBlock);
    free(self);
}

void dirtItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("DirtItemBlock_renderWorld");
void DirtItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(DirtItemBlock);
    itemBlockRenderWorld(&self->item_block, ctx, transforms);
}

void dirtItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("DirtItemBlock_renderInventory");
void DirtItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(DirtItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void dirtItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("DirtItemBlock_renderHand");
void DirtItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(DirtItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void dirtItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("DirtItemBlock_applyWorldRenderAttributes");
void DirtItemBlock_applyWorldRenderAttributes(VSelf) {

}

void dirtItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("DirtItemBlock_applyInventoryRenderAttributes");
void DirtItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(DirtItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void dirtItemBlockApplyHandRenderAttributes(VSelf) ALIAS("DirtItemBlock_applyHandRenderAttributes");
void DirtItemBlock_applyHandRenderAttributes(VSelf) {

}

void dirtItemBlockInit(VSelf) ALIAS("DirtItemBlock_init");
void DirtItemBlock_init(VSelf) {
    VSELF(DirtItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_DIRT,
            0,
            64
        ),
        .face_attributes = defaultFaceAttributes(2),
    };
}

void dirtItemBlockApplyDamage(VSelf) ALIAS("DirtItemBlock_applyDamage");
void DirtItemBlock_applyDamage(VSelf) {

}

void dirtItemBlockUseAction(VSelf) ALIAS("DirtItemBlock_useAction");
void DirtItemBlock_useAction(VSelf) {

}

void dirtItemBlockAttackAction(VSelf) ALIAS("DirtItemBlock_attackAction");
void DirtItemBlock_attackAction(VSelf) {

}
