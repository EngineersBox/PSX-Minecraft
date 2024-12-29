#include "block_stone.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/blocks/item_block_stone.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(stone, STONE)

void stoneBlockInit(VSelf) ALIAS("StoneBlock_init");
void StoneBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since StoneBlock composes Block as the first struct element
    VSELF(StoneBlock);
    self->block = declareSimpleBlock(BLOCKID_STONE);
}

IItem* stoneBlockDestroy(VSelf, bool drop_item) ALIAS("StoneBlock_destroy");
IItem* StoneBlock_destroy(VSelf, const bool drop_item) {
    VSELF(StoneBlock);
    return drop_item ? stoneBlockProvideItem(self) : NULL;
}

IItem* stoneBlockProvideItem(VSelf) ALIAS("StoneBlock_provideItem");
IItem* StoneBlock_provideItem(VSelf) {
    VSELF(StoneBlock);
    IItem* item = itemConstructor(stone)(0);
    StoneItemBlock* stone_item_block = VCAST_PTR(StoneItemBlock*, item);
    itemBlockReplicateFaceAttributes(stone_item_block->item_block, self->block);
    stone_item_block->item_block.item.stack_size = 1;
    stone_item_block->item_block.item.bob_direction = 1;
    return item;
}
