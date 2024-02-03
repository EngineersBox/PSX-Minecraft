#pragma once

#ifndef PSX_MINECRAFT_WORLD_H
#define PSX_MINECRAFT_WORLD_H

#include "position.h"
#include "../util/cvector.h"
#include "chunk/chunk.h"
#include "../render/render_context.h"
#include "../render/transforms.h"

#define WORLD_CHUNKS_HEIGHT 2
#define WORLD_HEIGHT (CHUNK_SIZE * WORLD_CHUNKS_HEIGHT)

// Must be positive
#define LOADED_CHUNKS_RADIUS 1
#define SHIFT_ZONE 1
#define CENTER 1
#if LOADED_CHUNKS_RADIUS < 1
#define AXIS_CHUNKS (CENTER + SHIFT_ZONE)
#else
#define AXIS_CHUNKS (((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * 2) + CENTER)
#endif

typedef struct World {
    VECTOR centre;
    struct {
        uint32_t vx;
        uint32_t vz;
    } head; // Top left, effective (0,0) of 2D array of chunks
    // TODO: Refactor chunks array from 3D -> 1D for better locality
    // X, Z, Y
    Chunk* chunks[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT];
} World;

void worldInit(World* world);
void worldDestroy(World* world);

void worldRender(const World* world, RenderContext* ctx, Transforms* transforms);

Chunk* worldLoadChunk(World* world, VECTOR chunk_position);
void worldUnloadChunk(const World* world, Chunk* chunk);
void worldLoadChunksX(World* world, int8_t x_direction, int8_t z_direction);
void worldLoadChunksZ(World* world, int8_t x_direction, int8_t z_direction);
void worldLoadChunksXZ(World* world, int8_t x_direction, int8_t z_direction);
void worldShiftChunks(World* world, int8_t x_direction, int8_t z_direction);
void worldLoadChunks(World* world, const VECTOR* player_chunk_pos);

void worldUpdate(World* world, const VECTOR* player_pos);

BlockID worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);
BlockID worldGetBlock(const World* world, const VECTOR* position);

#endif // PSX_MINECRAFT_WORLD_H
