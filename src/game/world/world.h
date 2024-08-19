#pragma once

#ifndef PSXMC_WORLD_H
#define PSXMC_WORLD_H

#include <stdbool.h>

#include "position.h"
#include "chunk/chunk.h"
#include "generation/chunk_provider.h"
#include "../render/render_context.h"
#include "../render/transforms.h"
#include "../entity/player.h"
#include "../util/inttypes.h"
#include "../blocks/breaking_state.h"
#include "../../lighting/lightmap.h"

#include "level/overworld_flatland.h"
#include "level/overworld_perlin.h"

typedef struct Weather {
    // Value in range [0, ONE] aka [0, 4096]
    fixedi16 rain_strength;
    // Value in range [0, ONE] aka [0, 4096]
    fixedi16 storm_strength;
    u32 rain_time_ticks;
    u32 storm_time_ticks;
    bool raining: 1;
    bool storming: 1;
    u8 _pad: 6;
} Weather;

#define WORLD_CHUNKS_HEIGHT 1
#define WORLD_HEIGHT (CHUNK_SIZE * WORLD_CHUNKS_HEIGHT)

// 20 min * 60 sec * 20 ticks
#define WORLD_TIME_CYCLE 24000
#define WORLD_TIME_NOON 6000
#define WORLD_TIME_DUSK 12000
#define WORLD_TIME_MIDNIGHT 18000
#define WORLD_TIME_DAWN 0

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
#define WORLD_CHUNKS_COUNT (AXIS_CHUNKS * AXIS_CHUNKS * WORLD_CHUNKS_HEIGHT)

typedef struct World {
    VECTOR centre;
    VECTOR centre_next;
    struct {
        u32 vx;
        u32 vz;
    } head; // Top left, effective (0,0) of 2D array of chunks
    LightLevel internal_light_level;
    u16 time_ticks;
    fixedi32 celestial_angle;
    Weather weather;
    IChunkProvider chunk_provider;
    // X, Z, Y
    Chunk* chunks[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT];
} World;

void worldInit(World* world, RenderContext* ctx);
void worldDestroy(World* world);

void worldRender(const World* world, RenderContext* ctx, Transforms* transforms);

void worldUnloadChunk(const World* world, Chunk* chunk);
ALLOC_CALL(worldUnloadChunk, 2) Chunk* worldLoadChunk(World* world, VECTOR chunk_position);
void worldLoadChunksX(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunksZ(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunksXZ(World* world, i8 x_direction, i8 z_direction);
void worldShiftChunks(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunks(World* world, const VECTOR* player_chunk_pos);

void worldUpdate(World* world, Player* player, BreakingState* breaking_state);

Chunk* worldGetChunkFromChunkBlock(const World* world, const ChunkBlockPosition* position);
Chunk* worldGetChunk(const World* world, const VECTOR* position);

IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);
IBlock* worldGetBlock(const World* world, const VECTOR* position);

bool worldModifyVoxelChunkBlock(const World* world,
                                const ChunkBlockPosition* position,
                                IBlock* block,
                                bool drop_item,
                                IItem** item_result);
IBlock* worldModifyVoxelChunkBlockConstructed(const World* world,
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
IBlock* worldModifyVoxelConstructed(const World* world,
                                    const VECTOR* position,
                                    BlockConstructor block_constructor,
                                    IItem* from_item,
                                    bool drop_item,
                                    IItem** item_result);

LightLevel worldGetLightType(const World* world,
                     const VECTOR* position,
                     const LightType light_type);
LightLevel worldGetLightTypeChunkBlock(const World* world,
                                       const ChunkBlockPosition* position,
                                       const LightType light_type);
LightLevel worldGetLightValue(const World* world,
                              const VECTOR* position);
LightLevel worldGetLightValueChunkBlock(const World* world,
                                        const ChunkBlockPosition* position);
void worldSetLightValue(const World* world,
                        const VECTOR* position,
                        const LightLevel light_value,
                        const LightType light_type);
void worldSetLightValueChunkBlock(const World* world,
                                  const ChunkBlockPosition* position,
                                  const LightLevel light_value,
                                  const LightType light_type);
void worldRemoveLightType(const World* world,
                          const VECTOR* position,
                          const LightType light_type);
void worldRemoveLightTypeChunkBlock(const World* world,
                                    const ChunkBlockPosition* position,
                                    const LightType light_type);

LightLevel worldGetInternalLightLevel(const World* world);

#endif // PSXMC_WORLD_H
