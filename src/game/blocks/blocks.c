#include "blocks.h"

uint8_t _last_block_index = 0;
Block BLOCKS[BLOCK_COUNT] = {};

// Stateless blocks
IBlock STONE_BLOCK_SINGLETON;

#define initBlockSingleton(instance, type) ({\
    instance##_SINGLETON = DYN_LIT(type, IBlock, {}); \
    VCALL(instance##_SINGLETON, init); \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(STONE_BLOCK, StoneBlock);

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
