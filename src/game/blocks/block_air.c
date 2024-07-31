#include "block_air.h"

#include <interface99.h>

#include "../../util/interface99_extensions.h"
#include "block_id.h"

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(air, AIR)

void airBlockInit(VSelf) ALIAS("AirBlock_init");
void AirBlock_init(VSelf) {
    VSELF(AirBlock);
    self->block = declareBlock(
        BLOCKID_AIR,
        0,
        0,
        0b000000,
        FACE_DIR_RIGHT,
        {}
    );
}

IItem* airBlockDestroy(VSelf, bool drop_item) ALIAS("AirBlock_destroy");
IItem* AirBlock_destroy(VSelf, const bool drop_item) {
    VSELF(AirBlock);
    return airBlockProvideItem(self);
}

IItem* airBlockProvideItem(VSelf) ALIAS("AirBlock_provideItem");
IItem* AirBlock_provideItem(VSelf) {
    return NULL;
}
