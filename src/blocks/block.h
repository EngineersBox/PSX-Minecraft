#pragma once

#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include "../primitive/cube.h"
#include "../core/display.h"
#include "../render/transforms.h"
#include "../primitive/primitive.h"

#define BLOCK_SIZE 25

typedef uint8_t BlockID;

typedef enum {
    AIR = 0,
    SOLID,
    STAIR,
    SLAB,
    CROSS
} BlockType;

typedef struct {
    BlockID id;
    BlockType type;
    char* name;
    Cube* cube;
} Block;

void blockRender(Block* block, DisplayContext* ctx, Transforms* transforms);

#define BLOCK_COUNT 256
extern const Block BLOCKS[];

#endif // PSX_MINECRAFT_BLOCK_H
