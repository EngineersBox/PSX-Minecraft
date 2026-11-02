#include "item_block_crafting_table.h"

#include <interface99.h>

#include "../../../util/interface99_extensions.h"
#include "../../../util/memory.h"
#include "../../../util/preprocessor.h"
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

void craftingTableItemBlockInit(VSelf) ALIAS("CraftingTableItemBlock_init");
void CraftingTableItemBlock_init(VSelf) {
    VSELF(CraftingTableItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_CRAFTING_TABLE),
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
