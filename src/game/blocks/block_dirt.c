#include "block_dirt.h"

#include "block_id.h"

DirtBlock* dirtBlockCreate() {
    return (DirtBlock*) &DIRT_BLOCK_SINGLETON;
}

void dirtBlockInit(VSelf) __attribute__((alias("DirtBlock_init")));
void DirtBlock_init(VSelf) {
    // TODO: Can make this VSELF(Block) since DirtBlock composes Block as the first struct element
    VSELF(DirtBlock);
    self->block = declareSolidBlock(
        BLOCKID_DIRT,
        "dirt",
        defaultFaceAttributes(2)
    );
}

void dirtBlockAccess(VSelf) __attribute__((alias("DirtBlock_access")));
void DirtBlock_access(VSelf) {
}

void dirtBlockDestroy(VSelf) __attribute__((alias("DirtBlock_destroy")));
void DirtBlock_destroy(VSelf) {
}

void dirtBlockUpdate(VSelf) __attribute__((alias("DirtBlock_update")));
void DirtBlock_update(VSelf) {
}
