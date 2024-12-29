#include "block_dirt.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/blocks/item_block_dirt.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(dirt, DIRT)

void dirtBlockInit(VSelf) ALIAS("DirtBlock_init");
void DirtBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since DirtBlock composes Block as the first struct element
    VSELF(DirtBlock);
    self->block = declareSimpleBlock(BLOCKID_DIRT);
}

IItem* dirtBlockDestroy(VSelf, bool drop_item) ALIAS("DirtBlock_destroy");
IItem* DirtBlock_destroy(VSelf, const bool drop_item) {
    VSELF(DirtBlock);
    return drop_item ? dirtBlockProvideItem(self) : NULL;
}

IItem* dirtBlockProvideItem(VSelf) ALIAS("DirtBlock_provideItem");
IItem* DirtBlock_provideItem(VSelf) {
    VSELF(DirtBlock);
    IItem* item = itemConstructor(dirt)(0);
    DirtItemBlock* dirt_item_block = VCAST_PTR(DirtItemBlock*, item);
    itemBlockReplicateFaceAttributes(dirt_item_block->item_block, self->block);
    dirt_item_block->item_block.item.stack_size = 1;
    dirt_item_block->item_block.item.bob_direction = 1;
    return item;
}
