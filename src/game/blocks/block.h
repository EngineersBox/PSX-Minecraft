
#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include <stdbool.h>
#include <interface99.h>

#include "../../util/inttypes.h"
#include "../../resources/texture.h"
#include "../../util/preprocessor.h"
#include "../items/item.h"

#define BLOCK_SIZE 70
#define BLOCK_FACES 6
#define BLOCK_TEXTURE_SIZE 16
#define ONE_BLOCK (BLOCK_SIZE << FIXED_POINT_SHIFT)

typedef struct {
    // Higher = more slip
    u16 slipperiness;
    // Higher = harder to destroy with a tool
    u16 hardness;
    // Higher = harder to destroy with TNT
    u16 resistance;
    char* name;
} BlockAttributes;

// ONE * 0.91 = 3727 == ice
// ONE * 0.6 = 2457 == normal block
#define BLOCK_DEFAULT_SLIPPERINESS 2457
// ONE * 0.5 * 5.0
#define BLOCK_DEFAULT_RESISTANCE 10240

typedef u8 BlockID;

typedef enum {
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

typedef enum {
    ORIENTATION_POS_X = 0,
    ORIENTATION_NEG_X,
    ORIENTATION_POS_Y,
    ORIENTATION_NEG_Y,
    ORIENTATION_POS_Z,
    ORIENTATION_NEG_Z
} Orientation;

typedef struct {
    BlockID id;
    u8 metadata_id;
    BlockType type; // TODO: Move this to attributes
    Orientation orientation;
    TextureAttributes face_attributes[BLOCK_FACES];
} Block;

#define opaqueFacesBitset(down, up, left, right, back, front) (\
      ((down) << 0) \
    | ((up) << 1) \
    | ((left) << 2) \
    | ((right) << 3) \
    | ((back) << 4) \
    | ((front) << 5) \
)

#define IBlock_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, access, VSelf) \
    vfunc(IItem*, destroy, VSelf) \
    vfunc(void, update, VSelf) \
    vfuncDefault(bool, isOpaque, VSelf, FaceDirection face_dir) \
    vfuncDefault(u8, opaqueBitset, VSelf) \
    vfunc(IItem*, provideItem, VSelf)

bool iBlockIsOpaque(VSelf, FaceDirection face_dir);
bool IBlock_isOpaque(VSelf, FaceDirection face_dir);

u8 iBlockOpaqueBitset(VSelf);
u8 IBlock_opaqueBitset(VSelf);

interface(IBlock);

#define DEFN_BLOCK_STATEFUL(name, ...) \
    typedef struct {\
        Block block; \
        __VA_ARGS__ \
    } name

#define DEFN_BLOCK_STATELESS(extern_name, name, ...) \
    DEFN_BLOCK_STATEFUL(name, P99_PROTECT(__VA_ARGS__)); \
    extern IBlock extern_name##_IBLOCK_SINGLETON; \
    extern name extern_name##_BLOCK_SINGLETON

#define declareBlock(_id, _metadata_id, _type, _orientation, _face_attributes) (Block) {\
    .id = (BlockID) _id,\
    .metadata_id = _metadata_id,\
    .type = (BlockType) _type,\
    .orientation = (Orientation) _orientation,\
    .face_attributes = _face_attributes\
}
#define declareFixedBlock(_id, _type, face_attributes) declareBlock( \
    _id, \
    0, \
    _type, \
    ORIENTATION_POS_X, \
    P99_PROTECT(face_attributes) \
)
#define declareFixedBlockMeta(_id, _metadata_id, _type, face_attributes) declareBlock( \
    _id, \
    _metadata_id, \
    _type, \
    ORIENTATION_POS_X, \
    P99_PROTECT(face_attributes) \
)
#define declareSolidBlock(_id, face_attributes) declareFixedBlock( \
    _id, \
    BLOCKTYPE_SOLID, \
    P99_PROTECT(face_attributes) \
)
#define declareSolidBlockMeta(_id, _metadata_id, face_attributes) declareFixedBlockMeta( \
    _id, \
    _metadata_id, \
    BLOCKTYPE_SOLID, \
    P99_PROTECT(face_attributes) \
)

#endif // PSX_MINECRAFT_BLOCK_H
