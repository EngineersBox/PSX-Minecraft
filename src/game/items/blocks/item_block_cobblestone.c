#include "item_block_cobblestone.h"

#include <stdlib.h>
#include <interface99.h>

#include "../../../util/interface99_extensions.h"
#include "../../../util/preprocessor.h"
#include "../../../math/vector.h"
#include "../item_id.h"

CobblestoneItemBlock* cobblestoneItemBlockCreate() {
    return (CobblestoneItemBlock*) malloc(sizeof(CobblestoneItemBlock));
}

DEFN_ITEM_CONSTRUCTOR(cobblestone) {
    IItem* item = itemCreate();
    CobblestoneItemBlock* cobblestone_item_block = cobblestoneItemBlockCreate();
    DYN_PTR(item, CobblestoneItemBlock, IItem, cobblestone_item_block);
    VCALL(*item, init);
    return item;
}

void cobblestoneItemBlockDestroy(VSelf) ALIAS("CobblestoneItemBlock_destroy");
void CobblestoneItemBlock_destroy(VSelf) {
    VSELF(CobblestoneItemBlock);
    free(self);
}

void cobblestoneItemBlockRenderWorld(VSelf,
                                     const Chunk* chunk,
                                     RenderContext* ctx,
                                     Transforms* transforms) ALIAS("CobblestoneItemBlock_renderWorld");
void CobblestoneItemBlock_renderWorld(VSelf,
                                      const Chunk* chunk,
                                      RenderContext* ctx,
                                      Transforms* transforms) {
    VSELF(CobblestoneItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
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
    UNIMPLEMENTED();
}

void cobblestoneItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("CobblestoneItemBlock_applyInventoryRenderAttributes");
void CobblestoneItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(CobblestoneItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void cobblestoneItemBlockApplyHandRenderAttributes(VSelf) ALIAS("CobblestoneItemBlock_applyHandRenderAttributes");
void CobblestoneItemBlock_applyHandRenderAttributes(VSelf) {
    UNIMPLEMENTED();
}

void cobblestoneItemBlockInit(VSelf) ALIAS("CobblestoneItemBlock_init");
void CobblestoneItemBlock_init(VSelf) {
    VSELF(CobblestoneItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_COBBLESTONE,
            0
        ),
        .face_attributes = defaultFaceAttributes(2),
    };
}

void cobblestoneItemBlockApplyDamage(VSelf) ALIAS("CobblestoneItemBlock_applyDamage");
void CobblestoneItemBlock_applyDamage(VSelf) {
    UNIMPLEMENTED();
}

void cobblestoneItemBlockUseAction(VSelf) ALIAS("CobblestoneItemBlock_useAction");
void CobblestoneItemBlock_useAction(VSelf) {
    UNIMPLEMENTED();
}

void cobblestoneItemBlockAttackAction(VSelf) ALIAS("CobblestoneItemBlock_attackAction");
void CobblestoneItemBlock_attackAction(VSelf) {
    UNIMPLEMENTED();
}
