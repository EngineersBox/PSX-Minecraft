#pragma once

#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include "../core/display.h"
#include "../render/transforms.h"
#include "../resources/assets.h"
#include "../util/preprocessor.h"

#define BLOCK_SIZE 50
#define BLOCK_FACES 6
#define BLOCK_TEXTURES 0
#define BLOCK_TEXTURE_SIZE 16

typedef uint8_t BlockID;

typedef enum _BlockType {
    BLOCKTYPE_EMPTY = 0,
    BLOCKTYPE_SOLID,
    BLOCKTYPE_STAIR,
    BLOCKTYPE_SLAB,
    BLOCKTYPE_CROSS,
    BLOCKTYPE_HASH
} BlockType;

typedef enum _Orientation {
    ORIENTATION_POS_X = 0,
    ORIENTATION_NEG_X,
    ORIENTATION_POS_Y,
    ORIENTATION_NEG_Y,
    ORIENTATION_POS_Z,
    ORIENTATION_NEG_Z
} Orientation;

typedef struct _Block {
    BlockID id;
    BlockType type;
    Orientation orientation;
    TextureAttributes faceAttributes[BLOCK_FACES];
    char* name;
} Block;

void blockRender(Block* block, DisplayContext* ctx, Transforms* transforms);

#define BLOCK_COUNT 256

typedef enum _BlockID {
    BLOCKID_NONE = -1,
    BLOCKID_AIR,
    BLOCKID_STONE,
    BLOCKID_DIRT,
    BLOCKID_GRASS
} EBlockID;

// Order
// - 0: -Z FRONT
// - 1: +Z BACK
// - 2: -Y TOP
// - 3: +Y BOTTOM
// - 4: -X LEFT
// - 5: +X RIGHT
#define declareSingleTextureFaceAttributes(neg_z, pos_z, neg_y, pos_y, neg_x, pos_x) { \
    {(neg_z) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, \
    {(pos_z) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, \
    {(neg_y) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, \
    {(pos_y) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, \
    {(neg_x) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, \
    {(pos_x) * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}} \
}
#define defaultFaceAttributes(index) declareSingleTextureFaceAttributes(index, index, index, index, index, index)

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

static const Block BLOCKS[BLOCK_COUNT] = {
    declareBlock(BLOCKID_AIR, "air", BLOCKTYPE_EMPTY, ORIENTATION_POS_X, {}),
    declareSolidBlock(BLOCKID_STONE, "stone", defaultFaceAttributes(1)),
    declareSolidBlock(BLOCKID_DIRT, "dirt", defaultFaceAttributes(2)),
    declareSolidBlock(BLOCKID_GRASS, "grass", declareSingleTextureFaceAttributes(3,3,2,0,3,3)),
};

#define blockAttribute(blockID, attr) (BLOCKS[(blockID)].attr)
#define blockIsOpaque(blockID) ((blockID) != BLOCKID_NONE && blockAttribute(blockID, type) != BLOCKTYPE_EMPTY)

#endif // PSX_MINECRAFT_BLOCK_H
