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

#define BLOCK_SIZE 50
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
    TextureAttributes faceAttributes[BLOCK_FACES];
    char* name;
} Block;

#define IBlock_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, access, VSelf) \
    vfunc(void, destroy, VSelf) \
    vfunc(void, update, VSelf) \
    vfuncDefault(bool, isOpaque, VSelf)

bool iBlockIsOpaque(VSelf);
bool IBlock_isOpaque(VSelf);

interface(IBlock);

#define DEFN_BLOCK(extern_name, name, ...) \
    typedef struct {\
        Block block; \
        __VA_ARGS__; \
    } name; \
    extern IBlock extern_name##_IBLOCK_SINGLETON; \
    extern name extern_name##_BLOCK_SINGLETON;

// Order
// - 0: -Z FRONT
// - 1: +Z BACK
// - 2: -Y TOP
// - 3: +Y BOTTOM
// - 4: -X LEFT
// - 5: +X RIGHT
#define declareTintedFaceAttributes(\
    neg_z, neg_z_tint, \
    pos_z, pos_z_tint, \
    neg_y, neg_y_tint, \
    pos_y, pos_y_tint, \
    neg_x, neg_x_tint, \
    pos_x, pos_x_tint \
) { \
    {(neg_z) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, neg_z_tint}, \
    {(pos_z) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, pos_z_tint}, \
    {(neg_y) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, neg_y_tint}, \
    {(pos_y) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, pos_y_tint}, \
    {(neg_x) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, neg_x_tint}, \
    {(pos_x) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, pos_x_tint} \
}
#define faceTint(r,g,b,cd) P99_PROTECT({r,g,b,cd})
#define NO_TINT faceTint(0,0,0,0)
#define declareFaceAttributes(neg_z, pos_z, neg_y, pos_y, neg_x, pos_x) declareTintedFaceAttributes( \
    neg_z, NO_TINT, \
    pos_z, NO_TINT, \
    neg_y, NO_TINT, \
    pos_y, NO_TINT, \
    neg_x, NO_TINT, \
    pos_x, NO_TINT \
)
#define defaultFaceAttributes(index) declareFaceAttributes(index, index, index, index, index, index)

#define declareBlock(_id, _name, _type, _orientation, face_attributes) (Block) {\
    .id = (BlockID) _id,\
    .type = (BlockType) _type,\
    .orientation = (Orientation) _orientation,\
    .faceAttributes = face_attributes,\
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

void blockRender(Block* block, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_BLOCK_H