#include "block_cobblestone.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/blocks/item_block_cobblestone.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(cobblestone, COBBLESTONE)

void cobblestoneBlockInit(VSelf) ALIAS("CobblestoneBlock_init");
void CobblestoneBlock_init(VSelf) {
    VSELF(CobblestoneBlock);
    self->block = declareSimpleBlock(BLOCKID_COBBLESTONE);
}

IItem* cobblestoneBlockDestroy(VSelf, bool drop_item) ALIAS("CobblestoneBlock_destroy");
IItem* CobblestoneBlock_destroy(VSelf, const bool drop_item) {
    VSELF(CobblestoneBlock);
    if (!drop_item) {
        return NULL;
    }
    return drop_item ? cobblestoneBlockProvideItem(self) : NULL;
}

IItem* cobblestoneBlockProvideItem(VSelf) ALIAS("CobblestoneBlock_provideItem");
IItem* CobblestoneBlock_provideItem(VSelf) {
    VSELF(CobblestoneBlock);
    IItem* item = itemConstructor(cobblestone)();
    CobblestoneItemBlock* cobblestone_item_block = VCAST_PTR(CobblestoneItemBlock*, item);
    itemBlockReplicateFaceAttributes(cobblestone_item_block->item_block, self->block);
    cobblestone_item_block->item_block.item.stack_size = 1;
    cobblestone_item_block->item_block.item.bob_direction = 1;
    return item;
}
