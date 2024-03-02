#include "block.h"

const char* EBLOCKID_NAMES[] = {
    MK_BLOCK_LIST(P99_STRING_ARRAY_INDEX)
};

uint8_t _last_block_index = 0;
Block BLOCKS[BLOCK_COUNT] = {};

void blockInitialiseBuiltin() {
    blockInitialise(declareBlock(BLOCKID_AIR, "air", BLOCKTYPE_EMPTY, ORIENTATION_POS_X, {}));
    blockInitialise(declareSolidBlock(BLOCKID_STONE, "stone", defaultFaceAttributes(1)));
    blockInitialise(declareSolidBlock(BLOCKID_DIRT, "dirt", defaultFaceAttributes(2)));
    blockInitialise(declareSolidBlock(
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
    ));
}

void blockRender(Block* block, RenderContext* ctx, Transforms* transforms) {

}
