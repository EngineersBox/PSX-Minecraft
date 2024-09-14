#include "block_test.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"
#include "../items/blocks/item_block_test.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(test, TEST)

void testBlockInit(VSelf) ALIAS("TestBlock_init");
void TestBlock_init(VSelf) {
    VSELF(TestBlock);
    self->block = declareBlock(
        BLOCKID_TEST,
        0,
        0,
        opacityBitset(1,0,0,0,0,0),
        FACE_DIR_RIGHT
    );
}

IItem* testBlockDestroy(VSelf, bool drop_item) ALIAS("TestBlock_destroy");
IItem* TestBlock_destroy(VSelf, bool drop_item) {
    VSELF(TestBlock);
    return drop_item ? testBlockProvideItem(self) : NULL;
}

IItem* testBlockProvideItem(VSelf) ALIAS("TestBlock_provideItem");
IItem* TestBlock_provideItem(VSelf) {
    VSELF(TestBlock);
    IItem* item = itemCreate();
    TestItemBlock* item_block = testItemBlockCreate();
    DYN_PTR(item, TestItemBlock, IItem, item_block);
    VCALL(*item, init);
    itemBlockReplicateFaceAttributes(item_block->item_block, self->block);
    item_block->item_block.item.stack_size = 1;
    item_block->item_block.item.bob_direction = 1;
    return item;
}

bool testBlockCanPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb) ALIAS("TestBlock_canPlace");
bool TestBlock_canPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb){
    UNIMPLEMENTED("Implement TestBlock_canPlace\n");
}
