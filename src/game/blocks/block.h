
#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include <stdbool.h>
#include <interface99.h>

#include "../../util/inttypes.h"
#include "../../resources/texture.h"
#include "../../util/preprocessor.h"
#include "../items/item.h"
#include "../items/tools/item_tool.h"
#include "../../structure/primitive/direction.h"

#define BLOCK_SIZE 70
#define BLOCK_FACES 6
#define BLOCK_TEXTURE_SIZE 16
#define ONE_BLOCK (BLOCK_SIZE << FIXED_POINT_SHIFT)

// ONE * 0.91 = 3727 == ice
// ONE * 0.6 = 2457 == normal block
#define BLOCK_DEFAULT_SLIPPERINESS 2457
// ONE * 0.5 * 5.0
#define BLOCK_DEFAULT_RESISTANCE 10240
// ONE * 0.5
#define BLOCK_DEFAULT_HARDNESS 2048

typedef u8 BlockID;

#define BLOCK_TYPE_COUNT 9
#define BLOCK_TYPE_COUNT_BITS 4
typedef enum BlockType {
    BLOCKTYPE_EMPTY = 0,
    // Regular block
    BLOCKTYPE_SOLID,
    // Stairs
    BLOCKTYPE_STAIR,
    // Slabs, bed, cake
    BLOCKTYPE_SLAB,
    // Spalings, flowers, reeds
    BLOCKTYPE_CROSS,
    // Wheat
    BLOCKTYPE_HASH,
    // Redstone, rails, door, trapdoor
    BLOCKTYPE_PLATE,
    // Torches
    BLOCKTYPE_ROD,
    // Water, lava
    BLOCKTYPE_LIQUID
} BlockType;

typedef struct BlockAttributes {
    // Higher = more slip
    u16 slipperiness;
    // Higher = harder to destroy with a tool
    u16 hardness;
    // Higher = harder to destroy with TNT
    u16 resistance;
    BlockType type: BLOCK_TYPE_COUNT_BITS;
    ToolType tool_type: TOOL_TYPE_COUNT_BITS;
    ItemMaterial tool_material: ITEM_MATERIAL_COUNT_BITS;
    u8 can_harvest: TOOL_TYPE_COUNT;
    u32 _pad: 15;
    char* name;
} BlockAttributes;

typedef struct Block {
    BlockID id;
    u8 metadata_id;
    u8 light_level: 4;
    u8 opacity_bitset: FACE_DIRECTION_COUNT;
    u8 _pad: 6;
    FaceDirection orientation;
    TextureAttributes face_attributes[BLOCK_FACES];
} Block;

#define BLOCK_DEFAULT_OPACITY_BITSET ((u8) 0b111111)
#define opacityBitset(down, up, left, right, back, front) (\
      ((down) << FACE_DIR_DOWN) \
    | ((up) << FACE_DIR_UP) \
    | ((left) << FACE_DIR_LEFT) \
    | ((right) << FACE_DIR_RIGHT) \
    | ((back) << FACE_DIR_BACK) \
    | ((front) << FACE_DIR_FRONT) \
)
#define blockIsFaceOpaque(block, face) (((block)->opacity_bitset >> (face)) & 0b1)

#define IBlock_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, access, VSelf) \
    vfunc(IItem*, destroy, VSelf, bool drop_item) \
    vfunc(void, update, VSelf) \
    vfuncDefault(bool, useAction, VSelf) \
    vfunc(IItem*, provideItem, VSelf)

bool iBlockUseAction(VSelf);
bool IBlock_useAction(VSelf);

interface(IBlock);

typedef IBlock* (*BlockConstructor)(IItem* from_item);

#define DEFN_BLOCK_STATEFUL(name, ...) \
    typedef struct {\
        Block block; \
        __VA_ARGS__ \
    } name

#define DEFN_BLOCK_STATELESS(name, extern_name, ...) \
    DEFN_BLOCK_STATEFUL(name, P99_PROTECT(__VA_ARGS__)); \
    extern IBlock extern_name##_IBLOCK_SINGLETON; \
    extern name extern_name##_BLOCK_SINGLETON

#define DEFN_BLOCK_CONSTRUCTOR(name) IBlock* name##BlockCreate(IItem* from_item)
#define DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(name, extern_name) DEFN_BLOCK_CONSTRUCTOR(name) { \
    if (from_item != NULL) { \
        Item* item = VCAST_PTR(Item*, from_item); \
        item->stack_size--; \
    } \
    return &extern_name##_IBLOCK_SINGLETON; \
}
#define DEFN_BLOCK_CONSTRUCTOR_IMPL_STATEFUL(name) DEFN_BLOCK_CONSTRUCTOR(name)

// Declare a Block instance
#define declareBlock(_id, _metadata_id, _light_level, _opacity_bitset, _orientation, _face_attributes) ((Block) {\
    .id = (BlockID) _id,\
    .metadata_id = _metadata_id,\
    .light_level = _light_level, \
    .opacity_bitset = _opacity_bitset, \
    .orientation = (FaceDirection) _orientation,\
    .face_attributes = _face_attributes\
})
// Declare a Block
#define declareSimpleBlock(_id, face_attributes) declareBlock( \
    _id, \
    0, \
    0, \
    BLOCK_DEFAULT_OPACITY_BITSET, \
    FACE_DIR_RIGHT, \
    P99_PROTECT(face_attributes) \
)
// Declare a Block with a light level
#define declareLightBlock(_id, _light_level) declareBlock( \
    _id, \
    0, \
    _light_level, \
    BLOCK_DEFAULT_OPACITY_BITSET, \
    FACE_DIR_RIGHT, \
    P99_PROTECT(face_attributes) \
)
// Declare a Block with a metadata ID
#define declareSimpleBlockMeta(_id, _metadata_id, face_attributes) declareBlock( \
    _id, \
    _metadata_id, \
    0, \
    BLOCK_DEFAULT_OPACITY_BITSET, \
    FACE_DIR_RIGHT, \
    P99_PROTECT(face_attributes) \
)

#endif // PSX_MINECRAFT_BLOCK_H
