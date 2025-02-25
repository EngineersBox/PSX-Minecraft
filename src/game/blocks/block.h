
#ifndef PSXMC_BLOCK_H
#define PSXMC_BLOCK_H

#include <stdbool.h>
#include <interface99.h>

#include "../../util/inttypes.h"
#include "../../resources/texture.h"
#include "../../util/preprocessor.h"
#include "../items/item.h"
#include "../items/tools/item_tool.h"
#include "../../structure/primitive/direction.h"
#include "../../physics/aabb.h"
#include "../../ui/ui.h"
#include "../../ui/components/background.h"

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

#define BLOCK_TYPE_COUNT 11
#define BLOCK_TYPE_COUNT_BITS 4
typedef enum BlockType {
    BLOCKTYPE_EMPTY = 0,
    // Regular block
    BLOCKTYPE_SOLID,
    // Stairs
    BLOCKTYPE_STAIR,
    // Slabs, bed, cake
    BLOCKTYPE_SLAB,
    // Fences
    BLOCKTYPE_FENCE,
    // Spalings, flowers, reeds
    BLOCKTYPE_CROSS,
    // Wheat
    BLOCKTYPE_HASH,
    // Redstone, rails, pressure plates
    BLOCKTYPE_PLATE,
    // Door, trapdoor
    BLOCKTYPE_DOOR,
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
    // Ideal tool to mine the block
    ToolType tool_type: TOOL_TYPE_COUNT_BITS;
    // Base material level of ideal tool to
    // drop something when mined
    ItemMaterial tool_material: ITEM_MATERIAL_COUNT_BITS;
    // Which tools can mine the block
    u8 can_harvest: TOOL_TYPE_COUNT;
    bool propagates_sunlight: 1;
    bool propagates_blocklight: 1;
    u16 _pad: 13;
    // This is a variable length array with length
    // a multiple of 6, where each 6 entries will
    // correspond to a metadata_id grouping of
    // textures
    TextureAttributes* face_attributes;
    char* name;
} BlockAttributes;

typedef struct Block {
    BlockID id;
    u8 metadata_id;
    u8 light_level: 4;
    FaceDirection orientation: FACE_DIRECTION_COUNT_BITS;
    u16 _pad: 9;
} Block;

FWD_DECL typedef struct World World;

#define BLOCK_USE_ACTION_CONSUMED true
#define BLOCK_USE_ACTION_NOT_CONUMED false

#define IBlock_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(IItem*, destroy, VSelf, bool drop_item) \
    /* Updates from world events like redstone */ \
    vfuncDefault(void, update, VSelf) \
    /* Player right clicking. True = action consumed, False = action not consumed */ \
    vfuncDefault(bool, useAction, VSelf) \
    /* Can block be placed */ \
    vfuncDefault(bool, canPlace, VSelf, const World* world, const VECTOR* position, const AABB* player_aabb) \
    /* Provide an item instance corresponding to this block */ \
    vfunc(IItem*, provideItem, VSelf)

void iblockUpdate(VSelf);
void IBlock_update(VSelf);

bool iBlockUseAction(VSelf);
bool IBlock_useAction(VSelf);

bool iBlockCanPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb);
bool IBlock_canPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb);

interface(IBlock);

typedef IBlock* (*BlockConstructor)(IItem* from_item, MAYBE_UNUSED u8 metadata_id);

#define DEFN_BLOCK_FACE_ATTRIBUTES(extern_name) \
    extern TextureAttributes extern_name##_FACE_ATTRIBUTES[]

#define DEFN_BLOCK_STATEFUL(name, extern_name, ...) \
    typedef struct {\
        Block block; \
        __VA_ARGS__ \
    } name; \
    DEFN_BLOCK_FACE_ATTRIBUTES(extern_name)

#define DEFN_BLOCK_METADATA_STATELESS(name, extern_name, metadata_id) \
    extern IBlock extern_name##_##metadata_id##_IBLOCK_SINGLETON; \
    extern name extern_name##_##metadata_id##_BLOCK_SINGLETON
#define DEFN_BLOCK_STATELESS(name, extern_name, ...) \
    DEFN_BLOCK_STATEFUL(name, P99_PROTECT(__VA_ARGS__)); \
    DEFN_BLOCK_METADATA_STATELESS(name, extern_name, 0)

#define DEFN_BLOCK_CONSTRUCTOR(name) IBlock* name##BlockCreate(IItem* from_item, MAYBE_UNUSED u8 metadata_id)
#define DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(name, extern_name) DEFN_BLOCK_CONSTRUCTOR(name) { \
    if (from_item != NULL) { \
        Item* item = VCAST_PTR(Item*, from_item); \
        item->stack_size--; \
    } \
    return &extern_name##_0_IBLOCK_SINGLETON; \
}
#define DEFN_BLOCK_CONSTRUCTOR_IMPL_STATEFUL(name) DEFN_BLOCK_CONSTRUCTOR(name)

// Declare a Block instance
#define declareBlock(_id, _metadata_id, _light_level, _orientation) ((Block) {\
    .id = (BlockID) _id,\
    .metadata_id = _metadata_id,\
    .light_level = _light_level, \
    .orientation = (FaceDirection) _orientation \
})
// Declare a Block
#define declareSimpleBlock(_id) declareBlock( \
    _id, \
    0, \
    0, \
    FACE_DIR_RIGHT \
)
// Declare a Block with a light level
#define declareLightBlock(_id, _light_level) declareBlock( \
    _id, \
    0, \
    _light_level, \
    FACE_DIR_RIGHT \
)
// Declare a Block with a metadata ID
#define declareSimpleBlockMeta(_id, _metadata_id) declareBlock( \
    _id, \
    _metadata_id, \
    0, \
    FACE_DIR_RIGHT \
)

// External data that blocks need when they need invoke
// a handler when a player interacts with it in some way
typedef struct BlockInputHandlerContext {
    IUI* inventory;
    IBlock* block;
} BlockInputHandlerContext;

extern BlockInputHandlerContext block_input_handler_context;

typedef void (*BlockRenderUIHandler)(RenderContext* ctx, Transforms* transforms);
typedef struct BlockRenderUIContext {
    BlockRenderUIHandler function;
    IBlock* block;
    UIBackground background;
} BlockRenderUIContext;

extern BlockRenderUIContext block_render_ui_context;

#endif // PSXMC_BLOCK_H
