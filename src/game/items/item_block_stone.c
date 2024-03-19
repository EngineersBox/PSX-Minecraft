#include "item_block_stone.h"

#include <stdlib.h>

#include "item_id.h"

StoneItemBlock* stoneItemBlockCreate() {
    return (StoneItemBlock*) malloc(sizeof(StoneItemBlock));
}

void stoneItemBlockDestroy(VSelf) __attribute__((alias("StoneItemBlock_destroy")));
void StoneItemBlock_destroy(VSelf) {
    VSELF(StoneItemBlock);
    free(self);
}

void stoneItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("StoneItemBlock_renderWorld")));
void StoneItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(StoneItemBlock);
    itemBlockRenderWorld(&self->item_block, ctx, transforms);
}

void stoneItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("StoneItemBlock_renderInventory")));
void StoneItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(StoneItemBlock);
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void stoneItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("StoneItemBlock_renderHand")));
void StoneItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(StoneItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void stoneItemBlockInit(VSelf) __attribute__((alias("StoneItemBlock_init")));
void StoneItemBlock_init(VSelf) {
    VSELF(StoneItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = ITEMID_STONE,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (SVECTOR) {0},
            .name = "stone",
            .picked_up = false
        },
        .face_attributes = defaultFaceAttributes(1),
    };
}

void stoneItemBlockApplyDamage(VSelf) __attribute__((alias("StoneItemBlock_applyDamage")));
void StoneItemBlock_applyDamage(VSelf) {

}

void stoneItemBlockUseAction(VSelf) __attribute__((alias("StoneItemBlock_useAction")));
void StoneItemBlock_useAction(VSelf) {

}

void stoneItemBlockAttackAction(VSelf) __attribute__((alias("StoneItemBlock_attackAction")));
void StoneItemBlock_attackAction(VSelf) {

}
