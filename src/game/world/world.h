#pragma once

#ifndef PSX_MINECRAFT_WORLD_H
#define PSX_MINECRAFT_WORLD_H

#include <stdbool.h>

#include "position.h"
#include "chunk/chunk.h"
#include "generation/chunk_provider.h"
#include "../render/render_context.h"
#include "../render/transforms.h"
#include "../entity/player.h"
#include "../util/inttypes.h"
#include "../blocks/breaking_state.h"

#include "level/overworld_flatland.h"
#include "level/overworld_perlin.h"

#define WORLD_CHUNKS_HEIGHT 1
#define WORLD_HEIGHT (CHUNK_SIZE * WORLD_CHUNKS_HEIGHT)

// TODO: Make these properties configurable as externs
//       to be accessible via some options interface
#ifndef LOADED_CHUNKS_RADIUS
// Must be positive
#define LOADED_CHUNKS_RADIUS 1
#endif
#define SHIFT_ZONE 1
#define CENTER 1
#define WORLD_CHUNKS_RADIUS (LOADED_CHUNKS_RADIUS + CENTER)
#if LOADED_CHUNKS_RADIUS < 1
#define AXIS_CHUNKS (CENTER + SHIFT_ZONE)
#else
#define AXIS_CHUNKS (((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * 2) + CENTER)
#endif

typedef struct World {
    VECTOR centre;
    struct {
        u32 vx;
        u32 vz;
    } head; // Top left, effective (0,0) of 2D array of chunks
    IChunkProvider chunk_provider;
    // TODO: Refactor chunks array from 3D -> 1D for better locality
    // X, Z, Y
    Chunk* chunks[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT];
} World;

void worldInit(World* world, RenderContext* ctx);
void worldDestroy(World* world);

void worldRender(const World* world, BreakingState* breaking_state, RenderContext* ctx, Transforms* transforms);

void worldUnloadChunk(const World* world, Chunk* chunk);
ALLOC_CALL(worldUnloadChunk, 2) Chunk* worldLoadChunk(World* world, VECTOR chunk_position);
void worldLoadChunksX(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunksZ(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunksXZ(World* world, i8 x_direction, i8 z_direction);
void worldShiftChunks(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunks(World* world, const VECTOR* player_chunk_pos);

void worldUpdate(World* world, Player* player);

Chunk* worldGetChunkFromChunkBlock(const World* world, const ChunkBlockPosition* position);
Chunk* worldGetChunk(const World* world, const VECTOR* position);

IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);
IBlock* worldGetBlock(const World* world, const VECTOR* position);

bool worldModifyVoxelChunkBlock(const World* world,
                                const ChunkBlockPosition* position,
                                IBlock* block,
                                bool drop_item,
                                IItem** item_result);
bool worldModifyVoxelChunkBlockConstructed(const World* world,
                                           const ChunkBlockPosition* position,
                                           BlockConstructor block_constructor,
                                           IItem* from_item,
                                           bool drop_item,
                                           IItem** item_result);

bool worldModifyVoxel(const World* world,
                      const VECTOR* position,
                      IBlock* block,
                      bool drop_item,
                      IItem** item_result);
bool worldModifyVoxelConstructed(const World* world,
                                 const VECTOR* position,
                                 BlockConstructor block_constructor,
                                 IItem* from_item,
                                 bool drop_item,
                                 IItem** item_result);

#endif // PSX_MINECRAFT_WORLD_H
