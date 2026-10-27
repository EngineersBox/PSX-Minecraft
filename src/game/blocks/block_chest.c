#include "block_chest.h"

#include "block_id.h"
#include "../items/blocks/item_block_chest.h"
#include "../../logging/logging.h"
#include "../../util/interface99_extensions.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATEFUL(chest) {
    TODO("Constructor for chest block");
}

void chestBlockInit(VSelf) ALIAS("ChestBlock_init");
void ChestBlock_init(VSelf) {
    VSELF(ChestBlock);
    self->block = declareBlock(BLOCKID_CHEST);
}

IItem* chestBlockDestroy(VSelf, bool drop_item, const VECTOR block_world_pos) ALIAS("ChestBlock_destroy");
IItem* ChestBlock_destroy(VSelf, bool drop_item, const VECTOR block_world_pos) {
    VSELF(ChestBlock);
    return drop_item ? chestBlockProvideItem(self) : NULL;
}

IItem* chestBlockProvideItem(VSelf) ALIAS("ChestBlock_provideItem");
IItem* ChestBlock_provideItem(VSelf) {
    VSELF(ChestBlock);
    IItem* item = itemConstructor(chest)(0);
    ChestItemBlock* item_block = VCAST_PTR(ChestItemBlock*, item);
    itemBlockReplicateFaceAttributes(item_block->item_block, self->block);
    item_block->item_block.item.stack_size = 1;
    item_block->item_block.item.bob_direction = 1;
    return item;
}

BlockUpdateResultBitmap chestBlockUpdate(VSelf, const VECTOR block_world_pos) ALIAS("ChestBlock_update");
BlockUpdateResultBitmap ChestBlock_update(VSelf, const VECTOR block_world_pos) {
    UNIMPLEMENTED();
}


bool chestBlockUseAction(VSelf) ALIAS("ChestBlock_useAction");
bool ChestBlock_useAction(VSelf) {
    UNIMPLEMENTED();
    return false;
}
