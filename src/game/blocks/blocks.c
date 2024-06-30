#include "blocks.h"

#include "../../util/interface99_extensions.h"
#include "../../logging/logging.h"

BlockAttributes block_attributes[BLOCK_COUNT] = {0};
BlockConstructor block_constructors[BLOCK_COUNT] = {0};

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
    block_attributes[(id)] = attributes; \
})

#define initBlockConstructor(id, constructor) ({ \
    block_constructors[(id)] = constructor; \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(AirBlock, AIR);
    initBlockAttributes(BLOCKID_AIR, airBlockCreateAttributes());
    initBlockConstructor(BLOCKID_AIR, airBlockCreate);

    initBlockSingleton(StoneBlock, STONE);
    initBlockAttributes(BLOCKID_STONE, stoneBlockCreateAttributes());
    initBlockConstructor(BLOCKID_STONE, stoneBlockCreate);

    initBlockSingleton(GrassBlock, GRASS);
    initBlockAttributes(BLOCKID_GRASS, grassBlockCreateAttributes());
    initBlockConstructor(BLOCKID_GRASS, grassBlockCreate);

    initBlockSingleton(DirtBlock, DIRT);
    initBlockAttributes(BLOCKID_DIRT, dirtBlockCreateAttributes());
    initBlockConstructor(BLOCKID_DIRT, dirtBlockCreate);

    initBlockSingleton(CobblestoneBlock, COBBLESTONE);
    initBlockAttributes(BLOCKID_COBBLESTONE, cobblestoneBlockCreateAttributes());
    initBlockConstructor(BLOCKID_COBBLESTONE, cobblestoneBlockCreate);
}

bool blockCanHarvest(const ToolType block_tool_type,
                     const ItemMaterial block_tool_material,
                     const ToolType item_tool_type,
                     const ItemMaterial item_tool_material,
                     const Block* block) {
    const bool can_harvest = blockGetItemCanHarvest(block->id, item_tool_type);
    if (!can_harvest) {
        return false;
    }
    if (item_tool_type == TOOLTYPE_NONE) {
        return true;
    }
    return block_tool_type == item_tool_type && item_tool_material >= block_tool_material;
}