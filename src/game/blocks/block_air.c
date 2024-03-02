#include "block_air.h"

#include "block_id.h"

AirBlock* airBlockCreate() {
    return (AirBlock*) &AIR_BLOCK_SINGLETON;
}

void airBlockInit(VSelf) __attribute__((alias("AirBlock_init")));
void AirBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since AirBlock composes Block as the first struct element
    VSELF(AirBlock);
    self->block = declareBlock(
        BLOCKID_AIR,
        "air",
        BLOCKTYPE_EMPTY,
        ORIENTATION_POS_X,
        {}
    );
}

void airBlockAccess(VSelf) __attribute__((alias("AirBlock_access")));
void AirBlock_access(VSelf) {
}

void airBlockDestroy(VSelf) __attribute__((alias("AirBlock_destroy")));
void AirBlock_destroy(VSelf) {
}

void airBlockUpdate(VSelf) __attribute__((alias("AirBlock_update")));
void AirBlock_update(VSelf) {
}