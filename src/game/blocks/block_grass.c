#include "block_grass.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/blocks/item_block_grass.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(grass, GRASS)

void grassBlockInit(VSelf) ALIAS("GrassBlock_init");
void GrassBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since GrassBlock composes Block as the first struct element
    VSELF(GrassBlock);
    self->block = declareSimpleBlock(BLOCKID_GRASS);
}

IItem* grassBlockDestroy(VSelf, bool drop_item) ALIAS("GrassBlock_destroy");
IItem* GrassBlock_destroy(VSelf, const bool drop_item) {
    VSELF(GrassBlock);
    return drop_item ? grassBlockProvideItem(self) : NULL;
}

IItem* grassBlockProvideItem(VSelf) ALIAS("GrassBlock_provideItem");
IItem* GrassBlock_provideItem(VSelf) {
    VSELF(GrassBlock);
    IItem* item = grassItemConstruct();
    GrassItemBlock* grass_item_block = VCAST_PTR(GrassItemBlock*, item);
    itemBlockReplicateFaceAttributes(grass_item_block->item_block, self->block);
    grass_item_block->item_block.item.stack_size = 1;
    grass_item_block->item_block.item.bob_direction = 1;
    return item;
}
