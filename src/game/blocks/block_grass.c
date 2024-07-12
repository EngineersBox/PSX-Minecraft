#include "block_grass.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/blocks/item_block_grass.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(grass, GRASS)

void grassBlockInit(VSelf) ALIAS("GrassBlock_init");
void GrassBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since GrassBlock composes Block as the first struct element
    VSELF(GrassBlock);
    self->block = declareSimpleBlock(
        BLOCKID_GRASS,
        declareTintedFaceAttributes(
            2 /*49*/, NO_TINT,
            0 /*49*/, /*NO_TINT,*/ faceTint(91, 139, 50, 1),
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT
        )
    );
}

void grassBlockAccess(VSelf) ALIAS("GrassBlock_access");
void GrassBlock_access(VSelf) {
}

IItem* grassBlockDestroy(VSelf, bool drop_item) ALIAS("GrassBlock_destroy");
IItem* GrassBlock_destroy(VSelf, const bool drop_item) {
    VSELF(GrassBlock);
    return drop_item ? grassBlockProvideItem(self) : NULL;
}

void grassBlockUpdate(VSelf) ALIAS("GrassBlock_update");
void GrassBlock_update(VSelf) {
}

IItem* grassBlockProvideItem(VSelf) ALIAS("GrassBlock_provideItem");
IItem* GrassBlock_provideItem(VSelf) {
    VSELF(GrassBlock);
    IItem* item = itemCreate();
    GrassItemBlock* grass_item_block = grassItemBlockCreate();
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    itemBlockReplicateFaceAttributes(grass_item_block->item_block, self->block);
    grass_item_block->item_block.item.stack_size = 64;
    grass_item_block->item_block.item.bob_direction = 1;
    return item;
}
