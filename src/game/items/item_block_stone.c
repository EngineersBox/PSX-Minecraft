#include "item_block_stone.h"

#include <stdlib.h>
#include <interface99.h>

StoneItemBlock* stoneItemBlockCreate() {
    return (StoneItemBlock*) malloc(sizeof(StoneItemBlock));
}

void someItemBlockDestroy(StoneItemBlock* stone_item_block) {
    free(stone_item_block);
}

void stoneItemBlockInit(VSelf) __attribute__((alias("StoneItemBlock_init")));
void StoneItemBlock_init(VSelf) {
    VSELF(StoneItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = 1,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (VECTOR) {0},
            .name = "stone"
        },
        .face_attributes = defaultFaceAttributes(1),
    };
}

void stoneItemBlockApplyDamage(VSelf) __attribute__((alias("StoneItemBlock_applyDamage")));
void StoneItemBlock_applyDamage(VSelf) {

}

void stoneItemBlockUseAction(VSelf) __attribute__((alias("StoneItemBlock_useAction")));
void StoneItemBlock_useAction(VSelf) {

}

void stoneItemBlockAttackAction(VSelf) __attribute__((alias("StoneItemBlock_attackAction")));
void StoneItemBlock_attackAction(VSelf) {

}
