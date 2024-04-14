#include "blocks.h"

#include <interface99_extensions.h>

BlockAttributes block_attributes[BLOCK_COUNT] = {0};

// Stateless blocks
#define _STATELESS_BLOCK(type, name) \
    type name##_BLOCK_SINGLETON = {}; \
    IBlock name##_IBLOCK_SINGLETON;

_STATELESS_BLOCK(AirBlock, AIR);
_STATELESS_BLOCK(StoneBlock, STONE);
_STATELESS_BLOCK(DirtBlock, DIRT);
_STATELESS_BLOCK(GrassBlock, GRASS);

#define initBlockSingleton(type, name) ({ \
    name##_IBLOCK_SINGLETON = DYN(type, IBlock, &name##_BLOCK_SINGLETON); \
    VCALL(name##_IBLOCK_SINGLETON, init); \
})

#define initBlockAttributes(id, ...) ({ \
    block_attributes[id] = P99_PROTECT(__VA_ARGS__); \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(AirBlock, AIR);
    initBlockAttributes(
        BLOCKID_AIR,
        (BlockAttributes) {
            .slipperiness = 0,
            .hardness = 0,
            .resistance = 0
        }
    );
    printf("Air: %d\n", VCAST(Block*, *airBlockCreate())->id);
    initBlockSingleton(StoneBlock, STONE);
    initBlockAttributes(
        BLOCKID_STONE,
        (BlockAttributes) {
            .slipperiness = 2457, // ONE * 0.6
            .hardness = 6144, // ONE * 1.5
            .resistance = 40960 // ONE * 10
        }
    );
    printf("Stone: %d\n", VCAST(Block*, *stoneBlockCreate())->id);
    initBlockSingleton(DirtBlock, DIRT);
    initBlockAttributes(
        BLOCKID_DIRT,
        (BlockAttributes) {
            .slipperiness = 2457, // ONE * 0.6
            .hardness = 2048, // ONE * 0.5
            .resistance = 10240 // ONE * 0.5 * 5.0 (default)
        }
    );
    printf("Dirt: %d\n", VCAST(Block*, *dirtBlockCreate())->id);
    initBlockSingleton(GrassBlock, GRASS);
    initBlockAttributes(
        BLOCKID_GRASS,
        (BlockAttributes) {
            .slipperiness = 2457, // ONE * 0.6
            .hardness = 2457, // ONE * 0.6
            .resistance = 12288 // ONE * 0.6 * 5.0 (default)
        }
    );
    printf("Grass: %d\n", VCAST(Block*, *grassBlockCreate())->id);
}
