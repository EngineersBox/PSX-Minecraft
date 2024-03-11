#include "block_stone.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/item_block_stone.h"

IBlock* stoneBlockCreate() {
    return &STONE_IBLOCK_SINGLETON;
}

void stoneBlockInit(VSelf) __attribute__((alias("StoneBlock_init")));
void StoneBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since StoneBlock composes Block as the first struct element
    VSELF(StoneBlock);
    self->block = declareSolidBlock(
        BLOCKID_STONE,
        "stone",
        defaultFaceAttributes(1)
    );
}

void stoneBlockAccess(VSelf) __attribute__((alias("StoneBlock_access")));
void StoneBlock_access(VSelf) {
}

void stoneBlockDestroy(VSelf, IItem* item_result) __attribute__((alias("StoneBlock_destroy")));
void StoneBlock_destroy(VSelf, IItem* item_result) {
    VSELF(StoneBlock);
    stoneBlockProvideItem(self, item_result);
}

void stoneBlockUpdate(VSelf) __attribute__((alias("StoneBlock_update")));
void StoneBlock_update(VSelf) {
}

void stoneBlockProvideItem(VSelf, IItem* item) __attribute__((alias("StoneBlock_provideItem")));
void StoneBlock_provideItem(VSelf, IItem* item) {
    if (item == NULL) {
        return;
    }
    VSELF(StoneBlock);
    StoneItemBlock* stone_item_block = stoneItemBlockCreate();
    DYN_PTR(item, StoneItemBlock, IItem, stone_item_block);
    VCALL(*item, init);
    itemBlockReplicateFaceAttributes(stone_item_block->item_block, self->block);
    stone_item_block->item_block.item.stack_size = 1;
    stone_item_block->item_block.item.bob_direction = 1;
}