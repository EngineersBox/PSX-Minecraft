#pragma once

#ifndef PSX_MINECRAFT_WORLD_H
#define PSX_MINECRAFT_WORLD_H

#include "position.h"
#include "../util/cvector.h"
#include "chunk/chunk.h"
#include "../core/display.h"
#include "../render/transforms.h"

// Must be positive
#define LOADED_CHUNKS_RADIUS 2
#define SHIFT_ZONE 1
#define CENTER 1
#if LOADED_CHUNKS_RADIUS <= 1
#define AXIS_CHUNKS (CENTER + SHIFT_ZONE)
#else
#define AXIS_CHUNKS (((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * 2) + CENTER)
#endif

// world = World*
#define chunkArrayCoord(world, axis, value) ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE + (world)->center_chunk.axis) + value)

typedef struct {
    VECTOR* center_chunk;
    // X, Z, Y
    Chunk _chunks[AXIS_CHUNKS][AXIS_CHUNKS][AXIS_CHUNKS];
    cvector(Chunk) chunks;
} World;

void worldInit(World* world);
void worldRender(const World* world, DisplayContext* ctx, Transforms* transforms);

void worldLoadChunks(World* world, const VECTOR* player_pos);

BlockID worldGetChunkBlock(const ChunkBlockPosition* position);
BlockID worldGetBlock(const VECTOR* position);

#endif // PSX_MINECRAFT_WORLD_H
