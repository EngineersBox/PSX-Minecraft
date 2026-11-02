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

void stoneItemBlockInit(VSelf) ALIAS("StoneItemBlock_init");
void StoneItemBlock_init(VSelf) {
    VSELF(StoneItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_STONE),
        .face_attributes = { defaultFaceAttributes(1) },
    };
}
