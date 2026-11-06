#include "item_block_chest.h"

#include <interface99_extensions.h>
#include "../../../core/std/stdlib.h"

#include "../item_id.h"
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

void chestItemBlockInit(VSelf) ALIAS("ChestItemBlock_init");
void ChestItemBlock_init(VSelf) {
    VSELF(ChestItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_CHEST),
        .face_attributes = { declareFaceAttributes(25,25,26,27,26,26) },
    };
}
