#include "item_block_grass.h"

#include <stdlib.h>

#include "item_id.h"

GrassItemBlock* grassItemBlockCreate() {
    return (GrassItemBlock*) malloc(sizeof(GrassItemBlock));
}

void grassItemBlockDestroy(VSelf) ALIAS("GrassItemBlock_destroy");
void GrassItemBlock_destroy(VSelf) {
    VSELF(GrassItemBlock);
    free(self);
}

void grassItemBlockRenderWorld(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("GrassItemBlock_renderWorld");
void GrassItemBlock_renderWorld(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderWorld(&self->item_block, ctx, transforms);
}

void grassItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("GrassItemBlock_renderInventory");
void GrassItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void grassItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("GrassItemBlock_renderHand");
void GrassItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void grassItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("GrassItemBlock_applyWorldRenderAttributes");
void GrassItemBlock_applyWorldRenderAttributes(VSelf) {

}

void grassItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("GrassItemBlock_applyInventoryRenderAttributes");
void GrassItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(GrassItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void grassItemBlockApplyHandRenderAttributes(VSelf) ALIAS("GrassItemBlock_applyHandRenderAttributes");
void GrassItemBlock_applyHandRenderAttributes(VSelf) {

}

void grassItemBlockInit(VSelf) ALIAS("GrassItemBlock_init");
void GrassItemBlock_init(VSelf) {
    VSELF(GrassItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = ITEMID_GRASS,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (SVECTOR) {0},
            .name = "grass",
            .picked_up = false
        },
        .face_attributes = declareTintedFaceAttributes(
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            2 /*49*/, NO_TINT,
            0 /*49*/, /*NO_TINT,*/ faceTint(0, 155, 0, 1),
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT
        )
    };
}

void grassItemBlockApplyDamage(VSelf) ALIAS("GrassItemBlock_applyDamage");
void GrassItemBlock_applyDamage(VSelf) {

}

void grassItemBlockUseAction(VSelf) ALIAS("GrassItemBlock_useAction");
void GrassItemBlock_useAction(VSelf) {

}

void grassItemBlockAttackAction(VSelf) ALIAS("GrassItemBlock_attackAction");
void GrassItemBlock_attackAction(VSelf) {

}
