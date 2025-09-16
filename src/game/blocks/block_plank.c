#include "block_plank.h"

#include "block_id.h"
#include "../items/blocks/item_block_plank.h"
#include "../../util/interface99_extensions.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(plank, PLANK)

void plankBlockInit(VSelf) ALIAS("PlankBlock_init");
void PlankBlock_init(VSelf) {
    VSELF(PlankBlock);
    self->block = declareSimpleBlock(BLOCKID_PLANK);
}

IItem* plankBlockDestroy(VSelf, bool drop_item) ALIAS("PlankBlock_destroy");
IItem* PlankBlock_destroy(VSelf, bool drop_item) {
    VSELF(PlankBlock);
    return drop_item ? plankBlockProvideItem(self) : NULL;
}

IItem* plankBlockProvideItem(VSelf) ALIAS("PlankBlock_provideItem");
IItem* PlankBlock_provideItem(VSelf) {
    VSELF(PlankBlock);
    IItem* item = itemConstructor(plank)(0);
    PlankItemBlock* item_block = VCAST_PTR(PlankItemBlock*, item);
    itemBlockReplicateFaceAttributes(item_block->item_block, self->block);
    item_block->item_block.item.stack_size = 1;
    item_block->item_block.item.bob_direction = 1;
    return item;
}
