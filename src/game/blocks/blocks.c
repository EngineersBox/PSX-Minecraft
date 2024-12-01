#include "blocks.h"

#include "../../util/interface99_extensions.h"
#include "../../logging/logging.h"
#include "block_air.h"
#include "block_cobblestone.h"
#include "block_dirt.h"
#include "block_grass.h"
#include "block_crafting_table.h"

BlockAttributes block_attributes[BLOCK_COUNT] = {0};
BlockConstructor block_constructors[BLOCK_COUNT] = {0};

// Stateless blocks
#define DECL_STATELESS_METADATA_BLOCK(type, extern_name, metadata_variant_count, face_attributes) \
    type extern_name##_BLOCK_SINGLETON = {}; \
    IBlock extern_name##_IBLOCK_SINGLETON; \
    TextureAttributes extern_name##_FACE_ATTRIBUTES[(metadata_variant_count) * FACE_DIRECTION_COUNT] = face_attributes

#define DECL_STATELESS_BLOCK(type, extern_name, face_attributes) \
    DECL_STATELESS_METADATA_BLOCK(type, extern_name, 1, P99_PROTECT(face_attributes))

DECL_STATELESS_BLOCK(AirBlock, AIR, airBlockFaceAttributes()); 
DECL_STATELESS_BLOCK(StoneBlock, STONE, stoneBlockFaceAttributes());
DECL_STATELESS_BLOCK(GrassBlock, GRASS, grassBlockFaceAttributes());
DECL_STATELESS_BLOCK(DirtBlock, DIRT, dirtBlockFaceAttributes());
DECL_STATELESS_BLOCK(CobblestoneBlock, COBBLESTONE, cobblestoneBlockFaceAttrbutes());

DECL_STATELESS_BLOCK(CraftingTableBlock, CRAFTING_TABLE, craftingTableBlockFaceAttributes());

#define initBlockStateful(id, attributes, constructor) ({ \
    block_attributes[(id)] = attributes; \
    block_constructors[(id)] = constructor; \
})

#define initBlockSingleton(type, extern_name, id, attributes, constructor) ({ \
    extern_name##_IBLOCK_SINGLETON = DYN(type, IBlock, &extern_name##_BLOCK_SINGLETON); \
    VCALL(extern_name##_IBLOCK_SINGLETON, init); \
    block_attributes[(id)] = attributes; \
    block_constructors[(id)] = constructor; \
})

void blocksInitialiseBuiltin() {
    initBlockSingleton(
        AirBlock, AIR, BLOCKID_AIR,
        airBlockCreateAttributes(), airBlockCreate
    );
    initBlockSingleton(
        StoneBlock, STONE, BLOCKID_STONE,
        stoneBlockCreateAttributes(), stoneBlockCreate
    );
    initBlockSingleton(
        GrassBlock, GRASS, BLOCKID_GRASS,
        grassBlockCreateAttributes(), grassBlockCreate
    );
    initBlockSingleton(
        DirtBlock, DIRT, BLOCKID_DIRT,
        dirtBlockCreateAttributes(), dirtBlockCreate
    );
    initBlockSingleton(
        CobblestoneBlock, COBBLESTONE, BLOCKID_COBBLESTONE,
        cobblestoneBlockCreateAttributes(), cobblestoneBlockCreate
    );
    initBlockSingleton(
        CraftingTableBlock, CRAFTING_TABLE, BLOCKID_CRAFTING_TABLE,
        craftingTableBlockCreateAttributes(), craftingTableBlockCreate
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
