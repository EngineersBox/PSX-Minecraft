#pragma once

#ifndef PSXMC_CHUNK_H
#define PSXMC_CHUNK_H

#include <psxgte.h>
#include <stdbool.h>

#include "chunk_structure.h"
#include "../../../render/render_context.h"
#include "../../../render/transforms.h"
#include "../../entity/player.h"
#include "../../../lighting/lightmap.h"
#include "../../blocks/blocks.h"
#include "../../items/item.h"
#include "../../blocks/breaking_state.h"

// ==== [NOTE] ====
// This class contains only methods and implementations
// for chunks. The actual defintion and defines associated
// with the chunk structure is in chunk_structure.h, the
// reason for this is to allow for easier inclusion of the
// structure without creating circular dependencies with
// the methods and implementations therein.

Chunk* chunkNew();
void chunkInit(Chunk* chunk);
void chunkDestroy(const Chunk* chunk);

void chunkGenerateMesh(Chunk* chunk);
void chunkClearMesh(Chunk* chunk);

// Initialise the lightmap, first by propogating sunlight
// and then applying any pending updates in the add light
// queue. This should be caled after generating a chunk.
void chunkGenerateLightmap(Chunk* chunk, ChunkGenerationContext* gen_ctx);
void chunkPropagateLightmap(Chunk* chunk, ChunkGenerationContext* gen_ctx);

/**
 * @brief Sets a block at a given location in the chunk, invoking the @code destroy@endcode
 *        method on any existing block at the location. If the destroyed block returns an item,
 *        it is added to the dropped items list on the chunk instance and a reference is set into
 *        the inout parameter item_result if it is not @code NULL@endcode. After being destroyed,
 *        the provided new block instance is set at the given position.
 * @param chunk Chunk instance
 * @param position Block position in local chunk coordinates in range (0,0,0) -> (7,7,7) inclusive
 * @param block Block instance to place at the given position
 * @param drop_item Whether to drop an item on invoking @code destroy@endcode
 * @param item_result Pointer to an item reference for the dropped item from destroying the block
 *                    that previously existed at the given location (if it exists). Supplying
 *                    @code NULL@endcode as a value to @code item_result@endcode will not return anything.
 * @return @code true@endcode if there was a previous block occupying the location or
 *         @code false@endcode if there was no block or the position it out of the chunk
 *         boundary.
 */
bool chunkModifyVoxel(Chunk* chunk,
                      const VECTOR* position,
                      IBlock* iblock,
                      bool drop_item,
                      IItem** item_result);
IBlock* chunkModifyVoxelConstructed(Chunk* chunk,
                                    const VECTOR* position,
                                    BlockConstructor block_constructor,
                                    IItem* from_item,
                                    bool drop_item,
                                    IItem** item_result);

void chunkRender(Chunk* chunk, bool subdivide, bool should_render, RenderContext* ctx, Transforms* transforms);

IBlock* chunkGetBlock(const Chunk* chunk, i32 x, i32 y, i32 z);
IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position);

void chunkUpdate(Chunk* chunk, const Player* player, BreakingState* breaking_state);

LightLevel chunkGetLightType(const Chunk* chunk,
                             const VECTOR* position,
                             const LightType light_type);
LightLevel chunkGetLightValue(const Chunk* chunk,
                              const VECTOR* position);
void chunkSetLightValue(Chunk* chunk,
                        const VECTOR* position,
                        const LightLevel light_value,
                        const LightType light_type);
void chunkRemoveLightValue(Chunk* chunk,
                           const VECTOR* position,
                           const LightType light_type);

void chunkUpdateLight(Chunk* chunk, const LightUpdateLimits limits);
void chunkUpdateAddLight(Chunk* chunk, const LightUpdateLimits limits);
void chunkUpdateRemoveLight(Chunk* chunk, const LightUpdateLimits limits);

#endif // PSXMC_CHUNK_H
