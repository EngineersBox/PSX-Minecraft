#include "item_block_cobblestone.h"

#include <stdlib.h>

#include "item_id.h"

CobblestoneItemBlock* cobblestoneItemBlockCreate() {
    return (CobblestoneItemBlock*) malloc(sizeof(CobblestoneItemBlock));
}

void cobblestoneItemBlockDestroy(VSelf) ALIAS("CobblestoneItemBlock_destroy");
void CobblestoneItemBlock_destroy(VSelf) {
    VSELF(CobblestoneItemBlock);
    free(self);
}

void cobblestoneItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CobblestoneItemBlock_renderWorld");
void CobblestoneItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CobblestoneItemBlock);
    itemBlockRenderWorld(&self->item_block, ctx, transforms);
}

void cobblestoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CobblestoneItemBlock_renderInventory");
void CobblestoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CobblestoneItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void cobblestoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CobblestoneItemBlock_renderHand");
void CobblestoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CobblestoneItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void cobblestoneItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("CobblestoneItemBlock_applyWorldRenderAttributes");
void CobblestoneItemBlock_applyWorldRenderAttributes(VSelf) {

}

void cobblestoneItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("CobblestoneItemBlock_applyInventoryRenderAttributes");
void CobblestoneItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(CobblestoneItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void cobblestoneItemBlockApplyHandRenderAttributes(VSelf) ALIAS("CobblestoneItemBlock_applyHandRenderAttributes");
void CobblestoneItemBlock_applyHandRenderAttributes(VSelf) {

}

void cobblestoneItemBlockInit(VSelf) ALIAS("CobblestoneItemBlock_init");
void CobblestoneItemBlock_init(VSelf) {
    VSELF(CobblestoneItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = ITEMID_COBBLESTONE,
            .metadata_id = 0,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (SVECTOR) {0},
            .name = "cobblestone",
            .picked_up = false
        },
        .face_attributes = defaultFaceAttributes(2),
    };
}

void cobblestoneItemBlockApplyDamage(VSelf) ALIAS("CobblestoneItemBlock_applyDamage");
void CobblestoneItemBlock_applyDamage(VSelf) {

}

void cobblestoneItemBlockUseAction(VSelf) ALIAS("CobblestoneItemBlock_useAction");
void CobblestoneItemBlock_useAction(VSelf) {

}

void cobblestoneItemBlockAttackAction(VSelf) ALIAS("CobblestoneItemBlock_attackAction");
void CobblestoneItemBlock_attackAction(VSelf) {

}