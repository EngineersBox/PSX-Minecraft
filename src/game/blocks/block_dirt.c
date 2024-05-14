#include "block_dirt.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/item_block_dirt.h"

IBlock* dirtBlockCreate() {
    return &DIRT_IBLOCK_SINGLETON;
}

void dirtBlockInit(VSelf) ALIAS("DirtBlock_init");
void DirtBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since DirtBlock composes Block as the first struct element
    VSELF(DirtBlock);
    self->block = declareSolidBlock(
        BLOCKID_DIRT,
        defaultFaceAttributes(2)
    );
}

void dirtBlockAccess(VSelf) ALIAS("DirtBlock_access");
void DirtBlock_access(VSelf) {
}

IItem* dirtBlockDestroy(VSelf) ALIAS("DirtBlock_destroy");
IItem* DirtBlock_destroy(VSelf) {
    VSELF(DirtBlock);
    return dirtBlockProvideItem(self);
}

void dirtBlockUpdate(VSelf) ALIAS("DirtBlock_update");
void DirtBlock_update(VSelf) {
}

IItem* dirtBlockProvideItem(VSelf) ALIAS("DirtBlock_provideItem");
IItem* DirtBlock_provideItem(VSelf) {
    VSELF(DirtBlock);
    IItem* item = itemCreate();
    DirtItemBlock* dirt_item_block = dirtItemBlockCreate();
    DYN_PTR(item, DirtItemBlock, IItem, dirt_item_block);
    VCALL(*item, init);
    itemBlockReplicateFaceAttributes(dirt_item_block->item_block, self->block);
    dirt_item_block->item_block.item.stack_size = 1;
    dirt_item_block->item_block.item.bob_direction = 1;
    return item;
}