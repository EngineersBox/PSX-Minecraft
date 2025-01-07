#include "item_block_grass.h"

#include <interface99.h>
#include <stdlib.h>

#include "../../../util/interface99_extensions.h"
#include "../item_id.h"
#include "item_block.h"

GrassItemBlock* grassItemBlockCreate() {
    return (GrassItemBlock*) malloc(sizeof(GrassItemBlock));
}

DEFN_ITEM_CONSTRUCTOR(grass) {
    IItem* item = itemCreate();
    GrassItemBlock* grass_item_block = grassItemBlockCreate();
    grass_item_block->item_block.item.metadata_id = 0;
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    return item;
}

void grassItemBlockDestroy(VSelf) ALIAS("GrassItemBlock_destroy");
void GrassItemBlock_destroy(VSelf) {
    VSELF(GrassItemBlock);
    free(self);
}

void grassItemBlockRenderWorld(VSelf,
                               const Chunk* chunk,
                               RenderContext* ctx,
                               Transforms* transforms) ALIAS("GrassItemBlock_renderWorld");
void GrassItemBlock_renderWorld(VSelf,
                                const Chunk* chunk,
                                RenderContext* ctx,
                                Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
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

/*void grassItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("GrassItemBlock_applyWorldRenderAttributes");*/
/*void GrassItemBlock_applyWorldRenderAttributes(VSelf) {*/
/*    VSELF(GrassItemBlock);*/
/*    itemBlockApplyWorldRenderAttributes(&self->item_block);*/
/*}*/
/**/
/*void grassItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("GrassItemBlock_applyInventoryRenderAttributes");*/
/*void GrassItemBlock_applyInventoryRenderAttributes(VSelf) {*/
/*    VSELF(GrassItemBlock);*/
/*    itemBlockApplyInventoryRenderAttributes(&self->item_block);*/
/*}*/
/**/
/*void grassItemBlockApplyHandRenderAttributes(VSelf) ALIAS("GrassItemBlock_applyHandRenderAttributes");*/
/*void GrassItemBlock_applyHandRenderAttributes(VSelf) {*/
/**/
/*}*/

void grassItemBlockInit(VSelf) ALIAS("GrassItemBlock_init");
void GrassItemBlock_init(VSelf) {
    VSELF(GrassItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_GRASS,
            0
        ),
        .face_attributes = { declareTintedFaceAttributes(
            2 /*49*/, NO_TINT,
            0 /*49*/, /*NO_TINT,*/ faceTint(91, 139, 50, 1),
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT
        ) }
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
