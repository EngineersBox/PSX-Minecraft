#include "item_block_plank.h"

#include <stdlib.h>
#include <interface99_extensions.h>

#include "../item_id.h"
#include "../../../logging/logging.h"
#include "../../../math/vector.h"
#include "../../../util/preprocessor.h"
#include "../../../util/memory.h"

PlankItemBlock* plankItemBlockCreate() {
    PlankItemBlock* itemblock = malloc(sizeof(PlankItemBlock));
    zeroed(itemblock);
    return itemblock;
}

DEFN_ITEM_CONSTRUCTOR(plank) {
    IItem* item = itemCreate();
    PlankItemBlock* plank_item_block = plankItemBlockCreate();
    plank_item_block->item_block.item.metadata_id = metadata_id;
    DYN_PTR(item, PlankItemBlock, IItem, plank_item_block);
    VCALL(*item, init);
    return item;
}

void plankItemBlockDestroy(VSelf) ALIAS("PlankItemBlock_destroy");
void PlankItemBlock_destroy(VSelf) {
    VSELF(PlankItemBlock);
    free(self);
}

void plankItemBlockRenderWorld(VSelf, const Chunk* chunk,  RenderContext* ctx, Transforms* transforms) ALIAS("PlankItemBlock_renderWorld");
void PlankItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    VSELF(PlankItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void plankItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("PlankItemBlock_renderInventory");
void PlankItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(PlankItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void plankItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("PlankItemBlock_renderHand");
void PlankItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(PlankItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void plankItemBlockApplyWorldRenderAttributes(VSelf) ALIAS("PlankItemBlock_applyWorldRenderAttributes");
void PlankItemBlock_applyWorldRenderAttributes(UNUSED VSelf) {
    UNIMPLEMENTED();
}

void plankItemBlockApplyInventoryRenderAttributes(VSelf) ALIAS("PlankItemBlock_applyInventoryRenderAttributes");
void PlankItemBlock_applyInventoryRenderAttributes(VSelf) {
    VSELF(PlankItemBlock);
    itemBlockApplyInventoryRenderAttributes(&self->item_block);
}

void plankItemBlockApplyHandRenderAttributes(VSelf) ALIAS("PlankItemBlock_applyHandRenderAttributes");
void PlankItemBlock_applyHandRenderAttributes(UNUSED VSelf) {
    UNIMPLEMENTED();
}

void plankItemBlockInit(VSelf) ALIAS("PlankItemBlock_init");
void PlankItemBlock_init(VSelf) {
    VSELF(PlankItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_PLANK),
        .face_attributes = { defaultFaceAttributes(4) },
    };
}
