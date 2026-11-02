#include "item_block_plank.h"

#include <interface99_extensions.h>
#include "../../../core/std/stdlib.h"

#include "../item_id.h"
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

void plankItemBlockInit(VSelf) ALIAS("PlankItemBlock_init");
void PlankItemBlock_init(VSelf) {
    VSELF(PlankItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_PLANK),
        .face_attributes = { defaultFaceAttributes(4) },
    };
}
