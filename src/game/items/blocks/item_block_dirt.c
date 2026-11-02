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

void dirtItemBlockInit(VSelf) ALIAS("DirtItemBlock_init");
void DirtItemBlock_init(VSelf) {
    VSELF(DirtItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_DIRT),
        .face_attributes = { defaultFaceAttributes(2) }
    };
}
