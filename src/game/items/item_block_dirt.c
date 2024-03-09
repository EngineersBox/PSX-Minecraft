#include "item_block_dirt.h"

#include <stdlib.h>

DirtItemBlock* dirtItemBlockCreate() {
    return (DirtItemBlock*) malloc(sizeof(DirtItemBlock));
}

void dirtItemBlockDestroy(DirtItemBlock* dirt_item_block) {
    free(dirt_item_block);
}

void dirtItemBlockInit(VSelf) __attribute__((alias("DirtItemBlock_init")));
void DirtItemBlock_init(VSelf) {
    VSELF(DirtItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = 3,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (VECTOR) {0},
            .name = "dirt"
        },
        .face_attributes = defaultFaceAttributes(2),
    };
}

void dirtItemBlockApplyDamage(VSelf) __attribute__((alias("DirtItemBlock_applyDamage")));
void DirtItemBlock_applyDamage(VSelf) {

}

void dirtItemBlockUseAction(VSelf) __attribute__((alias("DirtItemBlock_useAction")));
void DirtItemBlock_useAction(VSelf) {

}

void dirtItemBlockAttackAction(VSelf) __attribute__((alias("DirtItemBlock_attackAction")));
void DirtItemBlock_attackAction(VSelf) {

}
