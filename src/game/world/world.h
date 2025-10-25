#pragma once

#include "chunk/heightmap.h"
#ifndef PSXMC_WORLD_H
#define PSXMC_WORLD_H

#include <stdbool.h>

#include "position.h"
#include "chunk/chunk.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "../entity/player.h"
#include "../../util/inttypes.h"
#include "../blocks/breaking_state.h"
#include "world_defines.h"
#include "world_structure.h"

#include "level/overworld_flatland.h"
#include "level/overworld_perlin.h"

World* worldNew();
void worldInit(World* world, RenderContext* ctx);
void worldDestroy(World* world);

void worldRender(const World* world, const Player* player, RenderContext* ctx, Transforms* transforms);

void worldUnloadChunk(const World* world, Chunk* chunk);
ALLOC_CALL(worldUnloadChunk, 2) Chunk* worldLoadChunk(World* world, VECTOR chunk_position);
void worldLoadChunksX(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunksZ(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunksXZ(World* world, i8 x_direction, i8 z_direction);
void worldShiftChunks(World* world, i8 x_direction, i8 z_direction);
void worldLoadChunks(World* world, const VECTOR* player_chunk_pos);

void worldUpdate(World* world, Player* player, BreakingState* breaking_state, RenderContext* ctx);

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

// This is a transient pointer that can change after an update cycle,
// as such it should not be persisted.
ChunkHeightmap* worldGetChunkHeightmap(World* world, const VECTOR* position);
Heightmap* worldGetHeightmap(World* world);

// Count == 0 implies drop all
void worldDropItemStack(World* world, IItem* item, const u8 count);

#endif // PSXMC_WORLD_H
