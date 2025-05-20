#include "item_block_dirt.h"

#include <interface99.h>
#include "../../../core/std/stdlib.h"

#include "../../../util/interface99_extensions.h"
#include "../../../util/memory.h"
#include "../item_id.h"
#include "item_block.h"

DirtItemBlock* dirtItemBlockCreate() {
    DirtItemBlock* itemblock = malloc(sizeof(DirtItemBlock));
    zeroed(itemblock);
    return itemblock;
}

DEFN_ITEM_CONSTRUCTOR(dirt) {
    IItem* item = itemCreate();
    DirtItemBlock* dirt_item_block = dirtItemBlockCreate();
    dirt_item_block->item_block.item.metadata_id = 0;
    DYN_PTR(item, DirtItemBlock, IItem, dirt_item_block);
    VCALL(*item, init);
    return item;
}

void dirtItemBlockDestroy(VSelf) ALIAS("DirtItemBlock_destroy");
void DirtItemBlock_destroy(VSelf) {
    VSELF(DirtItemBlock);
    free(self);
}

void dirtItemBlockRenderWorld(VSelf,
                              const Chunk* chunk,
                              RenderContext* ctx,
                              Transforms* transforms) ALIAS("DirtItemBlock_renderWorld");
void DirtItemBlock_renderWorld(VSelf,
                               const Chunk* chunk,
                               RenderContext* ctx,
                               Transforms* transforms) {
    VSELF(DirtItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
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

void dirtItemBlockInit(VSelf) ALIAS("DirtItemBlock_init");
void DirtItemBlock_init(VSelf) {
    VSELF(DirtItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_DIRT,
            0
        ),
        .face_attributes = { defaultFaceAttributes(2) }
    };
}
