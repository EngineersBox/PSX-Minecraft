#include "block_air.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"

IBlock* airBlockCreate() {
    return &AIR_IBLOCK_SINGLETON;
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

bool airBlockIsOpaque(VSelf) __attribute__((alias("AirBlock_isOpaque")));
bool AirBlock_isOpaque(VSelf) {
    return false;
}