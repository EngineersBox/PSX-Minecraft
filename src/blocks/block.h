#pragma once

#ifndef PSX_MINECRAFT_BLOCK_H
#define PSX_MINECRAFT_BLOCK_H

#include "../primitive/cube.h"
#include "../core/display.h"
#include "../render/transforms.h"
#include "../primitive/primitive.h"

typedef struct {
    short id;
    char* name;
    Cube* cube;
} Block;

void blockRender(Block* block, DisplayContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_BLOCK_H
