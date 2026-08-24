#include "item_block_furnace.h"

#include <stdlib.h>
#include <interface99_extensions.h>

#include "../item_id.h"
#include "../../../util/preprocessor.h"
#include "../../../util/memory.h"

FurnaceItemBlock* furnaceItemBlockCreate() {
    FurnaceItemBlock* itemblock = malloc(sizeof(FurnaceItemBlock));
    zeroed(itemblock);
    return itemblock;
}

DEFN_ITEM_CONSTRUCTOR(furnace) {
    IItem* item = itemCreate();
    FurnaceItemBlock* furnace_item_block = furnaceItemBlockCreate();
    furnace_item_block->item_block.item.metadata_id = metadata_id;
    DYN_PTR(item, FurnaceItemBlock, IItem, furnace_item_block);
    VCALL(*item, init);
    return item;
}

void furnaceItemBlockDestroy(VSelf) ALIAS("FurnaceItemBlock_destroy");
void FurnaceItemBlock_destroy(VSelf) {
    VSELF(FurnaceItemBlock);
    free(self);
}

void furnaceItemBlockRenderWorld(VSelf, const Chunk* chunk,  RenderContext* ctx, Transforms* transforms) ALIAS("FurnaceItemBlock_renderWorld");
void FurnaceItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    VSELF(FurnaceItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void furnaceItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("FurnaceItemBlock_renderInventory");
void FurnaceItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(FurnaceItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void furnaceItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("FurnaceItemBlock_renderHand");
void FurnaceItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(FurnaceItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void furnaceItemBlockInit(VSelf) ALIAS("FurnaceItemBlock_init");
void FurnaceItemBlock_init(VSelf) {
    VSELF(FurnaceItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_FURNACE),
        .face_attributes = { declareTintedFaceAttributes(
            62, NO_TINT,
            62, NO_TINT,
            45, NO_TINT,
            45, NO_TINT,
            44, NO_TINT,
            45, NO_TINT
        ) },

    };
}
