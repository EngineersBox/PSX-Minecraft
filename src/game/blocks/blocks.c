#include "blocks.h"

#include "../../util/interface99_extensions.h"
#include "../../logging/logging.h"
#include "block.h"
#include "block_plank.h"

BlockAttributes block_attributes[BLOCK_COUNT] = {0};
// TODO: I think this array should be constant and initialised
//       at compile time. We know all of the ctor's ahead of time
//       so there is no point in leaving it to runtime init with
//       the helper macros. A bit more dev pain, but worth the
//       cost to allow the compiler to inline all usages of array 
//       elements at compile time.
BlockConstructor block_constructors[BLOCK_COUNT] = {0};           
                                                                  
#define ALL_FACE_DIRECTIONS_TRANSPARENT { \
    [FACE_DIR_DOWN]=opacityBitset(0, 0, 0, 0, 0, 0), \
    [FACE_DIR_UP]=opacityBitset(0, 0, 0, 0, 0, 0), \
    [FACE_DIR_LEFT]=opacityBitset(0, 0, 0, 0, 0, 0), \
    [FACE_DIR_RIGHT]=opacityBitset(0, 0, 0, 0, 0, 0), \
    [FACE_DIR_BACK]=opacityBitset(0, 0, 0, 0, 0, 0), \
    [FACE_DIR_FRONT]=opacityBitset(0, 0, 0, 0, 0, 0), \
}
#define ALL_FACE_DIRECTIONS_OPAQUE { \
    [FACE_DIR_DOWN]=opacityBitset(1, 1, 1, 1, 1, 1), \
    [FACE_DIR_UP]=opacityBitset(1, 1, 1, 1, 1, 1), \
    [FACE_DIR_LEFT]=opacityBitset(1, 1, 1, 1, 1, 1), \
    [FACE_DIR_RIGHT]=opacityBitset(1, 1, 1, 1, 1, 1), \
    [FACE_DIR_BACK]=opacityBitset(1, 1, 1, 1, 1, 1), \
    [FACE_DIR_FRONT]=opacityBitset(1, 1, 1, 1, 1, 1), \
}
u8 block_type_opacity_bitset[BLOCK_TYPE_COUNT][FACE_DIRECTION_COUNT] = {
    [BLOCKTYPE_EMPTY]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_SOLID]=ALL_FACE_DIRECTIONS_OPAQUE,
    [BLOCKTYPE_STAIR]={
        // Invalid
        [FACE_DIR_DOWN]=opacityBitset(1, 1, 1, 1, 1, 1),
        // Invalid
        [FACE_DIR_UP]=opacityBitset(1, 1, 1, 1, 1, 1),
        // Valid for the rest
        [FACE_DIR_LEFT]=opacityBitset(1, 0, 0, 1, 0, 0),
        [FACE_DIR_RIGHT]=opacityBitset(1, 0, 1, 0, 0, 0),
        [FACE_DIR_BACK]=opacityBitset(1, 0, 0, 0, 0, 1),
        [FACE_DIR_FRONT]=opacityBitset(1, 0, 0, 0, 1, 0),
    },
    [BLOCKTYPE_SLAB]={
        // Placed on top half of block
        [FACE_DIR_DOWN]=opacityBitset(0, 1, 0, 0, 0, 0),
        // Placed on bottom half of block
        [FACE_DIR_UP]=opacityBitset(1, 0, 0, 0, 0, 0),
        // Invalid for the rest
        [FACE_DIR_LEFT]=opacityBitset(1, 1, 1, 1, 1, 1),
        [FACE_DIR_RIGHT]=opacityBitset(1, 1, 1, 1, 1, 1),
        [FACE_DIR_BACK]=opacityBitset(1, 1, 1, 1, 1, 1),
        [FACE_DIR_FRONT]=opacityBitset(1, 1, 1, 1, 1, 1),
    },
    [BLOCKTYPE_FENCE]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_CROSS]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_HASH]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_PLATE]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_DOOR]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_ROD]=ALL_FACE_DIRECTIONS_TRANSPARENT,
    [BLOCKTYPE_LIQUID]=ALL_FACE_DIRECTIONS_TRANSPARENT,
};
#undef ALL_FACE_DIRECTIONS_TRANSPARENT
#undef ALL_FACE_DIRECTIONS_OPAQUE

// ==== START WRAPPER MACROS ====

#define DECL_STATELESS_METADATA_BLOCK(type, extern_name, metadata_id) \
    type extern_name##_##metadata_id##_BLOCK_SINGLETON = {0}; \
    IBlock extern_name##_##metadata_id##_IBLOCK_SINGLETON = {0}

#define DECL_STATELESS_BLOCK(type, extern_name, metadata_variant_count, face_attributes) \
    DECL_STATELESS_METADATA_BLOCK(type, extern_name, 0); \
    TextureAttributes extern_name##_FACE_ATTRIBUTES[(metadata_variant_count) * FACE_DIRECTION_COUNT] = face_attributes

#define initBlockStateful(id, attributes, constructor) ({ \
    block_attributes[(id)] = attributes; \
    block_constructors[(id)] = constructor; \
})

#define initBlockMetadataSingleton(type, extern_name, metadata_id) ({ \
    extern_name##_##metadata_id##_IBLOCK_SINGLETON = DYN( \
        type, \
        IBlock, \
        &extern_name##_##metadata_id##_BLOCK_SINGLETON \
    ); \
    VCALL(extern_name##_##metadata_id##_IBLOCK_SINGLETON, init); \
})

#define initBlockSingleton(type, extern_name, attributes, constructor) ({ \
    initBlockMetadataSingleton(type, extern_name, 0); \
    block_attributes[BLOCKID_##extern_name] = attributes; \
    block_constructors[BLOCKID_##extern_name] = constructor; \
})

// ==== END WRAPPER MACROS ====

DECL_STATELESS_BLOCK(AirBlock, AIR, 1, airBlockFaceAttributes()); 
DECL_STATELESS_BLOCK(StoneBlock, STONE, 1, stoneBlockFaceAttributes());
DECL_STATELESS_BLOCK(GrassBlock, GRASS, 1, grassBlockFaceAttributes());
DECL_STATELESS_BLOCK(DirtBlock, DIRT, 1, dirtBlockFaceAttributes());
DECL_STATELESS_BLOCK(CobblestoneBlock, COBBLESTONE, 1, cobblestoneBlockFaceAttrbutes());
DECL_STATELESS_BLOCK(PlankBlock, PLANK, 1, plankBlockFaceAttributes());
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
        PlankBlock, PLANK,
        plankBlockCreateAttributes(), plankBlockCreate
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
