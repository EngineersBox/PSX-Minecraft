#include "blocks.h"

#include <interface99_extensions.h>

BlockAttributes block_attributes[BLOCK_COUNT] = {0};

// Stateless blocks
#define DECL_STATELESS_BLOCK(type, name) \
    type name##_BLOCK_SINGLETON = {}; \
    IBlock name##_IBLOCK_SINGLETON;

DECL_STATELESS_BLOCK(AirBlock, AIR);
DECL_STATELESS_BLOCK(StoneBlock, STONE);
DECL_STATELESS_BLOCK(DirtBlock, DIRT);
DECL_STATELESS_BLOCK(GrassBlock, GRASS);

#define initBlockSingleton(type, name) ({ \
    name##_IBLOCK_SINGLETON = DYN(type, IBlock, &name##_BLOCK_SINGLETON); \
    VCALL(name##_IBLOCK_SINGLETON, init); \
})

#define initBlockAttributes(id, attributes) ({ \
    block_attributes[id] = attributes; \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(AirBlock, AIR);
    initBlockAttributes(
        BLOCKID_AIR,
        airBlockCreateAttributes()
    );
    printf("Air: %d\n", VCAST(Block*, *airBlockCreate())->id);
    initBlockSingleton(StoneBlock, STONE);
    initBlockAttributes(
        BLOCKID_STONE,
        stoneBlockCreateAttributes()
    );
    printf("Stone: %d\n", VCAST(Block*, *stoneBlockCreate())->id);
    initBlockSingleton(DirtBlock, DIRT);
    initBlockAttributes(
        BLOCKID_DIRT,
        dirtBlockCreateAttributes()
    );
    printf("Dirt: %d\n", VCAST(Block*, *dirtBlockCreate())->id);
    initBlockSingleton(GrassBlock, GRASS);
    initBlockAttributes(
        BLOCKID_GRASS,
        grassBlockCreateAttributes()
    );
    printf("Grass: %d\n", VCAST(Block*, *grassBlockCreate())->id);
}
