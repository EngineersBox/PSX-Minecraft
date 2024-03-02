#include "blocks.h"

uint8_t _last_block_index = 0;
Block BLOCKS[BLOCK_COUNT] = {};

// Stateless blocks
IBlock AIR_BLOCK_SINGLETON;
IBlock STONE_BLOCK_SINGLETON;
IBlock DIRT_BLOCK_SINGLETON;
IBlock GRASS_BLOCK_SINGLETON;

#define initBlockSingleton(instance, type) ({\
    instance##_BLOCK_SINGLETON = DYN_LIT(type, IBlock, {}); \
    VCALL(instance##_BLOCK_SINGLETON, init); \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(AIR, AirBlock);
    printf("Air: %d\n", VCAST(Block*, AIR_BLOCK_SINGLETON)->id);
    initBlockSingleton(STONE, StoneBlock);
    printf("Stone: %d\n", VCAST(Block*, STONE_BLOCK_SINGLETON)->id);
    initBlockSingleton(DIRT, DirtBlock);
    printf("Dirt: %d\n", VCAST(Block*, DIRT_BLOCK_SINGLETON)->id);
    initBlockSingleton(GRASS, GrassBlock);
    printf("Grass: %d\n", VCAST(Block*, GRASS_BLOCK_SINGLETON)->id);

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
