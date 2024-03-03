#include "block_grass.h"

#include "../../util/interface99_extensions.h"
#include "block_id.h"

IBlock* grassBlockCreate() {
    return &GRASS_IBLOCK_SINGLETON;
}

void grassBlockInit(VSelf) __attribute__((alias("GrassBlock_init")));
void GrassBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since GrassBlock composes Block as the first struct element
    VSELF(GrassBlock);
    self->block = declareSolidBlock(
        BLOCKID_GRASS,
        "grass",
        declareTintedFaceAttributes(
            3, NO_TINT,
            3, NO_TINT,
            2, NO_TINT,
            0, faceTint(0, 155, 0, 1),
            3, NO_TINT,
            3, NO_TINT
        )
    );
}

void grassBlockAccess(VSelf) __attribute__((alias("GrassBlock_access")));
void GrassBlock_access(VSelf) {
}

void grassBlockDestroy(VSelf) __attribute__((alias("GrassBlock_destroy")));
void GrassBlock_destroy(VSelf) {
}

void grassBlockUpdate(VSelf) __attribute__((alias("GrassBlock_update")));
void GrassBlock_update(VSelf) {
}
