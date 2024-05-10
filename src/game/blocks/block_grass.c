#include "block_grass.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/item_block_grass.h"

IBlock* grassBlockCreate() {
    return &GRASS_IBLOCK_SINGLETON;
}

void grassBlockInit(VSelf) __attribute__((alias("GrassBlock_init")));
void GrassBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since GrassBlock composes Block as the first struct element
    VSELF(GrassBlock);
    self->block = declareSolidBlock(
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

void grassBlockAccess(VSelf) __attribute__((alias("GrassBlock_access")));
void GrassBlock_access(VSelf) {
}

IItem* grassBlockDestroy(VSelf) __attribute__((alias("GrassBlock_destroy")));
IItem* GrassBlock_destroy(VSelf) {
    VSELF(GrassBlock);
    return grassBlockProvideItem(self);
}

void grassBlockUpdate(VSelf) __attribute__((alias("GrassBlock_update")));
void GrassBlock_update(VSelf) {
}

bool grassBlockIsOpaque(VSelf) __attribute__((alias("GrassBlock_isOpaque")));
bool GrassBlock_isOpaque(VSelf) {
    return true;
}

IItem* grassBlockProvideItem(VSelf) __attribute__((alias("GrassBlock_provideItem")));
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