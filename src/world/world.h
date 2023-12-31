#pragma once

#ifndef PSX_MINECRAFT_WORLD_H
#define PSX_MINECRAFT_WORLD_H

#include "../util/cvector.h"
#include "chunk/chunk.h"
#include "../core/display.h"
#include "../render/transforms.h"

typedef struct {
    cvector(Chunk) chunks;
} World;

void worldInit(World* world);
void worldRender(const World* world, DisplayContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_WORLD_H
