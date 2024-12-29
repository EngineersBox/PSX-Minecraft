#include "blocks.h"

#include "../../util/interface99_extensions.h"
#include "../../logging/logging.h"

BlockAttributes block_attributes[BLOCK_COUNT] = {0};
BlockConstructor block_constructors[BLOCK_COUNT] = {0};

#define DECL_STATELESS_METADATA_BLOCK(type, extern_name, metadata_id) \
    type extern_name##_##metadata_id##_BLOCK_SINGLETON = {}; \
    IBlock extern_name##_##metadata_id##_IBLOCK_SINGLETON;

#define DECL_STATELESS_BLOCK(type, extern_name, metadata_variant_count, face_attributes) \
    DECL_STATELESS_METADATA_BLOCK(type, extern_name, 0); \
    TextureAttributes extern_name##_FACE_ATTRIBUTES[(metadata_variant_count) * FACE_DIRECTION_COUNT] = face_attributes

#define initBlockStateful(id, attributes, constructor) ({ \
    block_attributes[(id)] = attributes; \
    block_constructors[(id)] = constructor; \
})

#define initBlockMetadataSingleton(type, extern_name, metadata_id) ({ \
    extern_name##_##metadata_id##_IBLOCK_SINGLETON = DYN(type, IBlock, &extern_name##_##metadata_id##_BLOCK_SINGLETON); \
    VCALL(extern_name##_##metadata_id##_IBLOCK_SINGLETON, init); \
})

#define initBlockSingleton(type, extern_name, attributes, constructor) ({ \
    initBlockMetadataSingleton(type, extern_name, 0); \
    block_attributes[BLOCKID_##extern_name] = attributes; \
    block_constructors[BLOCKID_##extern_name] = constructor; \
})

DECL_STATELESS_BLOCK(AirBlock, AIR, 1, airBlockFaceAttributes()); 
DECL_STATELESS_BLOCK(StoneBlock, STONE, 1, stoneBlockFaceAttributes());
DECL_STATELESS_BLOCK(GrassBlock, GRASS, 1, grassBlockFaceAttributes());
DECL_STATELESS_BLOCK(DirtBlock, DIRT, 1, dirtBlockFaceAttributes());
DECL_STATELESS_BLOCK(CobblestoneBlock, COBBLESTONE, 1, cobblestoneBlockFaceAttrbutes());
DECL_STATELESS_BLOCK(CraftingTableBlock, CRAFTING_TABLE, 1, craftingTableBlockFaceAttributes());

void blocksInitialiseBuiltin() {
    initBlockSingleton(
        AirBlock, AIR,
        airBlockCreateAttributes(), airBlockCreate
    );
    initBlockSingleton(
        StoneBlock, STONE,
        stoneBlockCreateAttributes(), stoneBlockCreate
    );
    initBlockSingleton(
        GrassBlock, GRASS,
        grassBlockCreateAttributes(), grassBlockCreate
    );
    initBlockSingleton(
        DirtBlock, DIRT,
        dirtBlockCreateAttributes(), dirtBlockCreate
    );
    initBlockSingleton(
        CobblestoneBlock, COBBLESTONE,
        cobblestoneBlockCreateAttributes(), cobblestoneBlockCreate
    );
    initBlockSingleton(
        CraftingTableBlock, CRAFTING_TABLE,
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
