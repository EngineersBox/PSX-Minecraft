#include "item_block_grass.h"

#include <stdlib.h>

GrassItemBlock* grassItemBlockCreate() {
    return (GrassItemBlock*) malloc(sizeof(GrassItemBlock));
}

void grassItemBlockDestroy(GrassItemBlock* grass_item_block) {
    free(grass_item_block);
}

void grassItemBlockInit(VSelf) __attribute__((alias("GrassItemBlock_init")));
void GrassItemBlock_init(VSelf) {
    VSELF(GrassItemBlock);
    self->item_block = (ItemBlock) {
        .item = (Item) {
            .id = 2,
            .type = ITEMTYPE_BLOCK,
            .stack_size = 0,
            .max_stack_size = 64,
            .position = (VECTOR) {0},
            .rotation = (VECTOR) {0},
            .name = "grass"
        },
        .face_attributes = declareTintedFaceAttributes(
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            2 /*49*/, NO_TINT,
            0 /*49*/, /*NO_TINT,*/ faceTint(0, 155, 0, 1),
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT
        )
    };
}

void grassItemBlockApplyDamage(VSelf) __attribute__((alias("GrassItemBlock_applyDamage")));
void GrassItemBlock_applyDamage(VSelf) {

}

void grassItemBlockUseAction(VSelf) __attribute__((alias("GrassItemBlock_useAction")));
void GrassItemBlock_useAction(VSelf) {

}

void grassItemBlockAttackAction(VSelf) __attribute__((alias("GrassItemBlock_attackAction")));
void GrassItemBlock_attackAction(VSelf) {

}
