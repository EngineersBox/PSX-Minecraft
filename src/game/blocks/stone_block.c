#include "stone_block.h"

#include "block_id.h"

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

void stoneBlockDestroy(VSelf) __attribute__((alias("StoneBlock_destroy")));
void StoneBlock_destroy(VSelf) {
}

void stoneBlockUpdate(VSelf) __attribute__((alias("StoneBlock_update")));
void StoneBlock_update(VSelf) {
}
