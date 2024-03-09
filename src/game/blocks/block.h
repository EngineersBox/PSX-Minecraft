#pragma once

#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <interface99.h>

#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "../../resources/assets.h"
#include "../../util/preprocessor.h"
#include "../items/item.h"

#define BLOCK_SIZE 70
#define BLOCK_FACES 6
#define BLOCK_TEXTURE_SIZE 16

typedef uint8_t BlockID;

typedef enum {
    BLOCKTYPE_EMPTY = 0,
    BLOCKTYPE_SOLID,
    BLOCKTYPE_STAIR,
    BLOCKTYPE_SLAB,
    BLOCKTYPE_CROSS,
    BLOCKTYPE_HASH
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
    BlockType type;
    Orientation orientation;
    TextureAttributes face_attributes[BLOCK_FACES];
    char* name;
} Block;

#define IBlock_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, access, VSelf) \
    vfunc(void, destroy, VSelf, IItem* item_result) \
    vfunc(void, update, VSelf) \
    vfuncDefault(bool, isOpaque, VSelf) \
    vfunc(void, provideItem, VSelf, IItem* item)

bool iBlockIsOpaque(VSelf);
bool IBlock_isOpaque(VSelf);

interface(IBlock);

#define DEFN_BLOCK_STATEFUL(name, ...) \
    typedef struct {\
        Block block; \
        __VA_ARGS__ \
    } name;

#define DEFN_BLOCK_STATELESS(extern_name, name, ...) \
    DEFN_BLOCK_STATEFUL(name, P99_PROTECT(__VA_ARGS__)) \
    extern IBlock extern_name##_IBLOCK_SINGLETON; \
    extern name extern_name##_BLOCK_SINGLETON;

#define declareBlock(_id, _name, _type, _orientation, _face_attributes) (Block) {\
    .id = (BlockID) _id,\
    .type = (BlockType) _type,\
    .orientation = (Orientation) _orientation,\
    .face_attributes = _face_attributes,\
    .name = _name\
}
#define declareFixedBlock(_id, _name, _type, face_attributes) declareBlock( \
    _id, \
    _name, \
    _type, \
    ORIENTATION_POS_X, \
    P99_PROTECT(face_attributes) \
)
#define declareSolidBlock(_id, _name, face_attributes) declareFixedBlock( \
    _id, \
    _name, \
    BLOCKTYPE_SOLID, \
    P99_PROTECT(face_attributes) \
)

#endif // PSX_MINECRAFT_BLOCK_H
