#include "item_block_chest.h"

#include <interface99_extensions.h>
#include "../../../core/std/stdlib.h"

#include "../item_id.h"
#include "../../../logging/logging.h"
#include "../../../math/vector.h"
#include "../../../util/preprocessor.h"
#include "../../../util/memory.h"

ChestItemBlock* chestItemBlockCreate() {
    ChestItemBlock* itemblock = malloc(sizeof(ChestItemBlock));
    zeroed(itemblock);
    return itemblock;
}

DEFN_ITEM_CONSTRUCTOR(chest) {
    IItem* item = itemCreate();
    ChestItemBlock* chest_item_block = chestItemBlockCreate();
    chest_item_block->item_block.item.metadata_id = metadata_id;
    DYN_PTR(item, ChestItemBlock, IItem, chest_item_block);
    VCALL(*item, init);
    return item;
}

void chestItemBlockDestroy(VSelf) ALIAS("ChestItemBlock_destroy");
void ChestItemBlock_destroy(VSelf) {
    VSELF(ChestItemBlock);
    free(self);
}

void chestItemBlockRenderWorld(VSelf, const Chunk* chunk,  RenderContext* ctx, Transforms* transforms) ALIAS("ChestItemBlock_renderWorld");
void ChestItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    VSELF(ChestItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void chestItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("ChestItemBlock_renderInventory");
void ChestItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(ChestItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void chestItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("ChestItemBlock_renderHand");
void ChestItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(ChestItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void chestItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("ChestItemBlock_applyWorldRenderAttributes");
void ChestItemBlock_applyWorldRenderAttributes(VSelf) {
    UNIMPLEMENTED();
}

void chestItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("ChestItemBlock_applyInventoryRenderAttributes");
void ChestItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(ChestItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void chestItemBlockApplyHandRenderAttributes(VSelf) ALIAS("ChestItemBlock_applyHandRenderAttributes");
void ChestItemBlock_applyHandRenderAttributes(VSelf) {
    UNIMPLEMENTED();
}

void chestItemBlockInit(VSelf) ALIAS("ChestItemBlock_init");
void ChestItemBlock_init(VSelf) {
    VSELF(ChestItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_CHEST),
        .face_attributes = { declareFaceAttributes(26,26,28,27,27,27) },

    };
}
