#pragma once

#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include "../core/display.h"
#include "../render/transforms.h"
#include "../resources/assets.h"
#include "../primitive/primitive.h"

#define BLOCK_SIZE 50
#define BLOCK_FACES 6
#define BLOCK_TEXTURES 0
#define BLOCK_TEXTURE_SIZE 16

typedef uint8_t BlockID;

typedef enum _BlockType {
    EMPTY = 0,
    SOLID,
    STAIR,
    SLAB,
    CROSS
} BlockType;

typedef struct _Block {
    BlockID id;
    BlockType type;
    TextureAttributes faceAttributes[BLOCK_FACES];
    char* name;
} Block;

void blockRender(Block* block, DisplayContext* ctx, Transforms* transforms);

#define BLOCK_COUNT 256

typedef enum _BlockID {
    NONE = -1,
    AIR,
    STONE,
    DIRT,
    GRASS
} EBlockID;


static const Block BLOCKS[BLOCK_COUNT] = {
    (Block) {
        .id = (BlockID) AIR,
        .type = (BlockType) EMPTY,
        .faceAttributes = {},
        .name = "air"
    },
    (Block) {
        .id = (BlockID) STONE,
        .type = SOLID,
        .faceAttributes = {
            {1 * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, // -Z FRONT
            {1 * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, // +Z BACK
            {1 * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, // -Y TOP
            {1 * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, // +Y BOTTOM
            {1 * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}, // -X LEFT
            {1 * BLOCK_TEXTURE_SIZE, 0, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE, {0}}  // +X RIGHT
        },
        .name = "stone"
    },
    (Block) {
        .id = (BlockID) DIRT,
        .type = SOLID,
        .faceAttributes = {
            {2 * 16, 0, 16, 16, {0}}, // -Z FRONT
            {2 * 16, 0, 16, 16, {0}}, // +Z BACK
            {2 * 16, 0, 16, 16, {0}}, // -Y TOP
            {2 * 16, 0, 16, 16, {0}}, // +Y BOTTOM
            {2 * 16, 0, 16, 16, {0}}, // -X LEFT
            {2 * 16, 0, 16, 16, {0}}  // +X RIGHT
        },
        .name = "dirt"
    },
    (Block) {
        .id = (BlockID) GRASS,
        .type = SOLID,
        .faceAttributes = {
            {3 * 16, 0, 16, 16, {0}}, // -Z FRONT
            {3 * 16, 0, 16, 16, {0}}, // +Z BACK
            {0 * 16, 0, 16, 16, {0, 155, 0, 1}}, // -Y TOP
            {2 * 16, 0, 16, 16, {0}}, // +Y BOTTOM
            {3 * 16, 0, 16, 16, {0}}, // -X LEFT
            {3 * 16, 0, 16, 16, {0}}  // +X RIGHT
        },
        .name = "grass"
    }
};

#define blockAttribute(block, attr) (BLOCKS[(Blocks) (block)].(attr))

#endif // PSX_MINECRAFT_BLOCK_H
