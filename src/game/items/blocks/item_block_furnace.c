#include "item_block_furnace.h"

#include <interface99_extensions.h>
#include "../../../core/std/stdlib.h"

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

void furnaceItemBlockInit(VSelf) ALIAS("FurnaceItemBlock_init");
void FurnaceItemBlock_init(VSelf) {
    VSELF(FurnaceItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_FURNACE),
        .face_attributes = { declareFaceAttributes(62,62,45,45,44,45) },
    };
}
