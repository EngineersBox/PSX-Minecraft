#include "blocks.h"

#include <interface99_extensions.h>

// Stateless blocks
#define DEFN_STATELESS_BLOCK(type, name) \
    type name##_BLOCK_SINGLETON = {}; \
    IBlock name##_IBLOCK_SINGLETON;

DEFN_STATELESS_BLOCK(AirBlock, AIR);
DEFN_STATELESS_BLOCK(StoneBlock, STONE);
DEFN_STATELESS_BLOCK(DirtBlock, DIRT);
DEFN_STATELESS_BLOCK(GrassBlock, GRASS);

#define initBlockSingleton(type, name) ({\
    name##_IBLOCK_SINGLETON = DYN(type, IBlock, &name##_BLOCK_SINGLETON); \
    VCALL(name##_IBLOCK_SINGLETON, init); \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(AirBlock, AIR);
    printf("Air: %d\n", VCAST(Block*, *airBlockCreate())->id);
    initBlockSingleton(StoneBlock, STONE);
    printf("Stone: %d\n", VCAST(Block*, *stoneBlockCreate())->id);
    initBlockSingleton(DirtBlock, DIRT);
    printf("Dirt: %d\n", VCAST(Block*, *dirtBlockCreate())->id);
    initBlockSingleton(GrassBlock, GRASS);
    printf("Grass: %d\n", VCAST(Block*, *grassBlockCreate())->id);
}
