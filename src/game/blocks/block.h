
#ifndef _PSXMC__GAME_BLOCKS__BLOCK_H_ 
#define _PSXMC__GAME_BLOCKS__BLOCK_H_

#include <stdbool.h>
#include <interface99.h>

#include "../items/item.h"
#include "../world/position.h"
#include "../../lighting/lightmap.h"
#include "../../physics/aabb.h"
#include "../../resources/texture.h"
#include "../../structure/primitive/direction.h"
#include "../../ui/ui.h"
#include "../../ui/components/background.h"
#include "../../util/inttypes.h"
#include "../../util/preprocessor.h"

// ==== BLOCK STRUCTURE ====

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
FWD_DECL typedef struct Chunk Chunk;

#define BLOCK_USE_ACTION_CONSUMED true
#define BLOCK_USE_ACTION_NOT_CONSUMED false

typedef enum BlockUpdateResult {
    // Keep block in chunk's update map/queue
    BLOCK_UPDATE_RESULT_PERSIST = 0,
    // Remove block from chunk's update map/queue
    BLOCK_UPDATE_RESULT_RELEASE = 1
} BlockUpdateResult;

#define IBlock_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(IItem*, destroy, VSelf, bool drop_item) \
    /* Updates from world events like redstone */ \
    vfuncDefault(BlockUpdateResult, update, VSelf) \
    /* Player right clicking. True = action consumed, False = action not consumed */ \
    vfuncDefault(bool, useAction, VSelf) \
    /* Can block be placed */ \
    vfuncDefault(bool, canPlace, VSelf, const World* world, const VECTOR* position, const AABB* player_aabb) \
    /* Provide an item instance corresponding to this block */ \
    vfunc(IItem*, provideItem, VSelf)

BlockUpdateResult iblockUpdate(VSelf);
BlockUpdateResult IBlock_update(VSelf);

bool iBlockUseAction(VSelf);
bool IBlock_useAction(VSelf);

bool iBlockCanPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb);
bool IBlock_canPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb);

interface(IBlock);

void iblockDestroy(IBlock* iblock);
ALLOC_CALL(iblockDestroy, 1) IBlock* iblockCreate();

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

// Declare a block instance, using
// designated initialisers for all
// optional fields (i.e. not 'id').
//
// For example:
// declareBlock(BLOCKID_EXAMPLE, .light_level = 5)
#define declareBlock(_id, ...) ((Block) {\
    .id = (BlockID) _id, \
    .metadata_id = 0, \
    .light_level = 0, \
    .orientation = (FaceDirection) FACE_DIR_RIGHT, \
    __VA_ARGS__ \
})

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

#define resetBlockRenderUIContext() ({\
    block_render_ui_context.function = NULL; \
    block_render_ui_context.block = NULL; \
    block_render_ui_context.background = (UIBackground) {0}; \
})

// ==== BLOCK UPDATES ====

typedef u8 BlockUpdateTypeBitmap;

#define blockUpdateTypeBitmapGet(bitmap, update_type) (((bitmap) >> (update_type)) & 0b1)
#define blockUpdateTypeBitmapSet(bitmap, update_type) ((bitmap) |= 1 << (update_type))
#define blockUpdateTypeBitmapUnset(bitmap, update_type) ((bitmap) &= ~(1 << (update_type)))

#define BLOCK_UPDATE_TYPE_COUNT 5
#define BLOCK_UPDATE_TYPE_BITS 3
typedef enum BlockUpdateType {
    BLOCK_UPDATE_TYPE_ADD_SKYLIGHT = 0,
    BLOCK_UPDATE_TYPE_REMOVE_SKYLIGHT = 1,
    BLOCK_UPDATE_TYPE_ADD_BLOCKLIGHT = 2,
    BLOCK_UPDATE_TYPE_REMOVE_BLOCKLIGHT = 3,
    BLOCK_UPDATE_TYPE_STATE = 4
} BlockUpdateType;

typedef struct BlockUpdate {
    ChunkBlockPosition position;
    BlockUpdateTypeBitmap type_bitmap: BLOCK_UPDATE_TYPE_BITS;
    u8 _pad: 8 - BLOCK_UPDATE_TYPE_BITS;
    LightLevel old_skylight_value;
    LightLevel old_block_light_value;
} BlockUpdate;

u64 blockUpdateHash(const void* item, u64 seed0, u64 seed1);
int blockUpdateCompare(const void* a, const void* b, UNUSED void* ignored);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_H_ 
