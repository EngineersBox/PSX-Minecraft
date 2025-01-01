#include "item_block_stone.h"

#include <interface99.h>
#include <stdlib.h>

#include "../../../util/interface99_extensions.h"
#include "../item_id.h"
#include "item_block.h"

StoneItemBlock* stoneItemBlockCreate() {
    return (StoneItemBlock*) malloc(sizeof(StoneItemBlock));
}

DEFN_ITEM_CONSTRUCTOR(stone) {
    IItem* item = itemCreate();
    StoneItemBlock* stone_item_block = stoneItemBlockCreate();
    stone_item_block->item_block.item.metadata_id = 0;
    DYN_PTR(item, StoneItemBlock, IItem, stone_item_block);
    VCALL(*item, init);
    return item;
}

void stoneItemBlockDestroy(VSelf) ALIAS("StoneItemBlock_destroy");
void StoneItemBlock_destroy(VSelf) {
    VSELF(StoneItemBlock);
    free(self);
}

void stoneItemBlockRenderWorld(VSelf,
                               const Chunk* chunk,
                               RenderContext* ctx,
                               Transforms* transforms) ALIAS("StoneItemBlock_renderWorld");
void StoneItemBlock_renderWorld(VSelf,
                                const Chunk* chunk,
                                RenderContext* ctx,
                                Transforms* transforms) {
    VSELF(StoneItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void stoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("StoneItemBlock_renderInventory");
void StoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(StoneItemBlock);
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void stoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("StoneItemBlock_renderHand");
void StoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(StoneItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void stoneItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("StoneItemBlock_applyWorldRenderAttributes");
void StoneItemBlock_applyWorldRenderAttributes(VSelf) {
    VSELF(StoneItemBlock);
    itemBlockApplyWorldRenderAttributes(&self->item_block);
}

void stoneItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("StoneItemBlock_applyInventoryRenderAttributes");
void StoneItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(StoneItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void stoneItemBlockApplyHandRenderAttributes(VSelf) ALIAS("StoneItemBlock_applyHandRenderAttributes");
void StoneItemBlock_applyHandRenderAttributes(VSelf) {

}

void stoneItemBlockInit(VSelf) ALIAS("StoneItemBlock_init");
void StoneItemBlock_init(VSelf) {
    VSELF(StoneItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_STONE,
            0
        ),
        .face_attributes = { defaultFaceAttributes(1) },
    };
}

void stoneItemBlockApplyDamage(VSelf) ALIAS("StoneItemBlock_applyDamage");
void StoneItemBlock_applyDamage(VSelf) {

}

void stoneItemBlockUseAction(VSelf) ALIAS("StoneItemBlock_useAction");
void StoneItemBlock_useAction(VSelf) {

}

void stoneItemBlockAttackAction(VSelf) ALIAS("StoneItemBlock_attackAction");
void StoneItemBlock_attackAction(VSelf) {

}
