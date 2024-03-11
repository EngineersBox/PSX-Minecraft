#include "block_dirt.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/item_block_dirt.h"

IBlock* dirtBlockCreate() {
    return &DIRT_IBLOCK_SINGLETON;
}

void dirtBlockInit(VSelf) __attribute__((alias("DirtBlock_init")));
void DirtBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since DirtBlock composes Block as the first struct element
    VSELF(DirtBlock);
    self->block = declareSolidBlock(
        BLOCKID_DIRT,
        "dirt",
        defaultFaceAttributes(2)
    );
}

void dirtBlockAccess(VSelf) __attribute__((alias("DirtBlock_access")));
void DirtBlock_access(VSelf) {
}

void dirtBlockDestroy(VSelf, IItem* item_result) __attribute__((alias("DirtBlock_destroy")));
void DirtBlock_destroy(VSelf, IItem* item_result) {
    VSELF(DirtBlock);
    dirtBlockProvideItem(self, item_result);
}

void dirtBlockUpdate(VSelf) __attribute__((alias("DirtBlock_update")));
void DirtBlock_update(VSelf) {
}

void dirtBlockProvideItem(VSelf, IItem* item) __attribute__((alias("DirtBlock_provideItem")));
void DirtBlock_provideItem(VSelf, IItem* item) {
    if (item == NULL) {
        return;
    }
    VSELF(DirtBlock);
    DirtItemBlock* dirt_item_block = dirtItemBlockCreate();
    DYN_PTR(item, DirtItemBlock, IItem, dirt_item_block);
    VCALL(*item, init);
    itemBlockReplicateFaceAttributes(dirt_item_block->item_block, self->block);
    dirt_item_block->item_block.item.stack_size = 1;
    dirt_item_block->item_block.item.bob_direction = 1;
}