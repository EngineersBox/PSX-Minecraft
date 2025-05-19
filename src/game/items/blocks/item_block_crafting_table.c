#include "item_block_crafting_table.h"

#include "../../../core/std/stdlib.h"
#include <interface99.h>

#include "../../../util/interface99_extensions.h"
#include "../../../util/memory.h"
#include "../../../util/preprocessor.h"
#include "../../../math/vector.h"
#include "../item_id.h"
#include "item_block.h"

CraftingTableItemBlock* craftingTableItemBlockCreate() {
    CraftingTableItemBlock* itemblock = malloc(sizeof(CraftingTableItemBlock));
    zeroed(itemblock);
    return itemblock;
}

DEFN_ITEM_CONSTRUCTOR(craftingTable) {
    IItem* item = itemCreate();
    CraftingTableItemBlock* crafting_table_item_block = craftingTableItemBlockCreate();
    crafting_table_item_block->item_block.item.metadata_id = 0;
    DYN_PTR(item, CraftingTableItemBlock, IItem, crafting_table_item_block);
    VCALL(*item, init);
    return item;
}

void craftingTableItemBlockDestroy(VSelf) ALIAS("CraftingTableItemBlock_destroy");
void CraftingTableItemBlock_destroy(VSelf) {
    VSELF(CraftingTableItemBlock);
    free(self);
}

void craftingTableItemBlockRenderWorld(VSelf, const Chunk* chunk,  RenderContext* ctx, Transforms* transforms) ALIAS("CraftingTableItemBlock_renderWorld");
void CraftingTableItemBlock_renderWorld(VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    VSELF(CraftingTableItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void craftingTableItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CraftingTableItemBlock_renderInventory");
void CraftingTableItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CraftingTableItemBlock);
    self->item_block.item.position.vz = ITEM_BLOCK_INVENTORY_SCALING;
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void craftingTableItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("CraftingTableItemBlock_renderHand");
void CraftingTableItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(CraftingTableItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void craftingTableItemBlockInit(VSelf) ALIAS("CraftingTableItemBlock_init");
void CraftingTableItemBlock_init(VSelf) {
    VSELF(CraftingTableItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareSimpleItem(
            ITEMID_CRAFTING_TABLE,
            0
        ),
        .face_attributes = { declareTintedFaceAttributes(
            4, NO_TINT,
            43, NO_TINT,
            59, NO_TINT,
            59, NO_TINT,
            60, NO_TINT,
            60, NO_TINT
        ) }
    };
}
