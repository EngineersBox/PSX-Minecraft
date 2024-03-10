#include "item_block_dirt.h"

#include <stdlib.h>

#include "item_id.h"

DirtItemBlock* dirtItemBlockCreate() {
    return (DirtItemBlock*) malloc(sizeof(DirtItemBlock));
}

void dirtItemBlockDestroy(DirtItemBlock* dirt_item_block) {
    free(dirt_item_block);
}

void dirtItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("DirtItemBlock_renderWorld")));
void DirtItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(DirtItemBlock);
    itemBlockRenderWorld(&self->item_block, ctx, transforms);
}

void dirtItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("DirtItemBlock_renderInventory")));
void DirtItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(DirtItemBlock);
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void dirtItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("DirtItemBlock_renderHand")));
void DirtItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(DirtItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void dirtItemBlockInit(VSelf) __attribute__((alias("DirtItemBlock_init")));
void DirtItemBlock_init(VSelf) {
    VSELF(DirtItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = ITEMID_DIRT,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (VECTOR) {0},
            .name = "dirt"
        },
        .face_attributes = defaultFaceAttributes(2),
    };
}

void dirtItemBlockApplyDamage(VSelf) __attribute__((alias("DirtItemBlock_applyDamage")));
void DirtItemBlock_applyDamage(VSelf) {

}

void dirtItemBlockUseAction(VSelf) __attribute__((alias("DirtItemBlock_useAction")));
void DirtItemBlock_useAction(VSelf) {

}

void dirtItemBlockAttackAction(VSelf) __attribute__((alias("DirtItemBlock_attackAction")));
void DirtItemBlock_attackAction(VSelf) {

}
