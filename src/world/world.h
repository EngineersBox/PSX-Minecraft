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
    VECTOR centre;
    struct {
        uint32_t vx;
        uint32_t vz;
    } head; // Top left, effective (0,0) of 2D array of chunks
    // X, Z, Y
    Chunk* _chunks[AXIS_CHUNKS][AXIS_CHUNKS][1];
    cvector(Chunk) chunks;
} World;

void worldInit(World* world);
void worldRender(const World* world, DisplayContext* ctx, Transforms* transforms);

Chunk* worldLoadChunk(VECTOR chunk_position);
Chunk* worldUnloadChunk(VECTOR chunk_position);
void worldLoadChunksX(World* world, int8_t x_direction, int8_t z_direction);
void worldLoadChunksZ(World* world, int8_t x_direction, int8_t z_direction);
void worldLoadChunksXZ(World* world, int8_t x_direction, int8_t z_direction);
void worldShiftChunks(World* world, int8_t x_direction, int8_t z_direction);
void worldLoadChunks(World* world, const VECTOR* player_pos);

BlockID worldGetChunkBlock(const ChunkBlockPosition* position);
BlockID worldGetBlock(const VECTOR* position);

#endif // PSX_MINECRAFT_WORLD_H
