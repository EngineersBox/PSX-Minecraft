#include "blocks.h"

#include "../../util/interface99_extensions.h"
#include "../../logging/logging.h"
#include "block_air.h"

BlockAttributes block_attributes[BLOCK_COUNT] = {0};
BlockConstructor block_constructors[BLOCK_COUNT] = {0};

// Stateless blocks
#define DECL_STATELESS_BLOCK(type, name) \
    type name##_BLOCK_SINGLETON = {}; \
    IBlock name##_IBLOCK_SINGLETON; \
    TextureAttributes* name##_FACE_ATTRIBUTES

DECL_STATELESS_BLOCK(AirBlock, AIR);
DECL_STATELESS_BLOCK(StoneBlock, STONE);
DECL_STATELESS_BLOCK(GrassBlock, GRASS);
DECL_STATELESS_BLOCK(DirtBlock, DIRT);
DECL_STATELESS_BLOCK(CobblestoneBlock, COBBLESTONE);

#define initBlockSingleton(type, extern_name, id, attributes, constructor, face_attributes) ({ \
    extern_name##_IBLOCK_SINGLETON = DYN(type, IBlock, &extern_name##_BLOCK_SINGLETON); \
    VCALL(extern_name##_IBLOCK_SINGLETON, init); \
    block_attributes[(id)] = attributes; \
    block_constructors[(id)] = constructor; \
    extern_name##_FACE_ATTRIBUTES = face_attributes; \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(
        AirBlock, AIR, BLOCKID_AIR,
        airBlockCreateAttributes(), airBlockCreate,
        NULL
    );
    initBlockSingleton(
        StoneBlock, STONE, BLOCKID_STONE,
        stoneBlockCreateAttributes(), stoneBlockCreate,
        NULL /* TODO: Implement this*/
    );
    initBlockSingleton(
        GrassBlock, GRASS, BLOCKID_GRASS,
        grassBlockCreateAttributes(), grassBlockCreate,
        NULL /* TODO: Implement this */
    );
    initBlockSingleton(
        DirtBlock, DIRT, BLOCKID_DIRT,
        dirtBlockCreateAttributes(), dirtBlockCreate,
        NULL /* TODO: Implement this */
    );
    initBlockSingleton(
        CobblestoneBlock, COBBLESTONE, BLOCKID_COBBLESTONE,
        cobblestoneBlockCreateAttributes(), cobblestoneBlockCreate,
        NULL /* TODO: Implement this */
    );
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
