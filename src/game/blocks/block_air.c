#include "block_air.h"

#include <interface99.h>

#include "../../util/interface99_extensions.h"
#include "block_id.h"

IBlock* airBlockCreate() {
    return &AIR_IBLOCK_SINGLETON;
}

void airBlockInit(VSelf) ALIAS("AirBlock_init");
void AirBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since AirBlock composes Block as the first struct element
    VSELF(AirBlock);
    self->block = declareBlock(
        BLOCKID_AIR,
        0,
        BLOCKTYPE_EMPTY,
        ORIENTATION_POS_X,
        {}
    );
}

void airBlockAccess(VSelf) ALIAS("AirBlock_access");
void AirBlock_access(VSelf) {
}

IItem* airBlockDestroy(VSelf) ALIAS("AirBlock_destroy");
IItem* AirBlock_destroy(VSelf) {
    VSELF(AirBlock);
    return airBlockProvideItem(self);
}

void airBlockUpdate(VSelf) ALIAS("AirBlock_update");
void AirBlock_update(VSelf) {
}

bool airBlockIsOpaque(VSelf, FaceDirection face_dir) ALIAS("AirBlock_isOpaque");
bool AirBlock_isOpaque(VSelf, UNUSED FaceDirection face_dir) {
    return false;
}

IItem* airBlockProvideItem(VSelf) ALIAS("AirBlock_provideItem");
IItem* AirBlock_provideItem(VSelf) {
    return NULL;
}