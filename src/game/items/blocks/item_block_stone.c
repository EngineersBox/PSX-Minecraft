#include "item_block_stone.h"

#include <interface99.h>
#include "../../../core/std/stdlib.h"

#include "../../../util/interface99_extensions.h"
#include "../../../util/memory.h"
#include "../item_id.h"
#include "item_block.h"

StoneItemBlock* stoneItemBlockCreate() {
    StoneItemBlock* itemblock = malloc(sizeof(StoneItemBlock));
    zeroed(itemblock);
    return itemblock;
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
