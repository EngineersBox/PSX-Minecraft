#include "blocks.h"

#include "../../util/interface99_extensions.h"
#include "../../logging/logging.h"

BlockAttributes block_attributes[BLOCK_COUNT] = {0};

// Stateless blocks
#define DECL_STATELESS_BLOCK(type, name) \
    type name##_BLOCK_SINGLETON = {}; \
    IBlock name##_IBLOCK_SINGLETON;

DECL_STATELESS_BLOCK(AirBlock, AIR);
DECL_STATELESS_BLOCK(StoneBlock, STONE);
DECL_STATELESS_BLOCK(GrassBlock, GRASS);
DECL_STATELESS_BLOCK(DirtBlock, DIRT);
DECL_STATELESS_BLOCK(CobblestoneBlock, COBBLESTONE);

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
    DEBUG_LOG("Air: %d\n", VCAST_PTR(Block*, airBlockCreate())->id);
    initBlockSingleton(StoneBlock, STONE);
    initBlockAttributes(
        BLOCKID_STONE,
        stoneBlockCreateAttributes()
    );
    DEBUG_LOG("Stone: %d\n", VCAST_PTR(Block*, stoneBlockCreate())->id);
    initBlockSingleton(GrassBlock, GRASS);
    initBlockAttributes(
        BLOCKID_GRASS,
        grassBlockCreateAttributes()
    );
    DEBUG_LOG("Grass: %d\n", VCAST_PTR(Block*, grassBlockCreate())->id);
    initBlockSingleton(DirtBlock, DIRT);
    initBlockAttributes(
        BLOCKID_DIRT,
        dirtBlockCreateAttributes()
    );
    DEBUG_LOG("Dirt: %d\n", VCAST_PTR(Block*, dirtBlockCreate())->id);
    initBlockSingleton(CobblestoneBlock, COBBLESTONE);
    initBlockAttributes(
        BLOCKID_COBBLESTONE,
        cobblestoneBlockCreateAttributes()
    );
    DEBUG_LOG("Dirt: %d\n", VCAST_PTR(Block*, cobblestoneBlockCreate())->id);
}
