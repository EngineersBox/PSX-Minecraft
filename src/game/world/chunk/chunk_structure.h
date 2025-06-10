#pragma once

#ifndef _PSXMC__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_
#define _PSXMC__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_

#include "../../../structure/cvector.h"
#include "../../../structure/hashmap.h"
#include "chunk_mesh.h"
#include "chunk_defines.h"
#include "chunk_visibility.h"
#include "../position.h"
#include "../../blocks/blocks.h"
#include "../../../lighting/lightmap.h"

// -1 = unlimited
typedef struct LightUpdateLimits {
    i16 add_block;
    i16 add_sky;
    i16 remove_block;
    i16 remove_sky;
} LightUpdateLimits;
extern const LightUpdateLimits chunk_light_update_limits;

#define chunkBlockIndex(x, y, z) ((z) + ((y) * CHUNK_SIZE) + ((x) * CHUNK_SIZE * CHUNK_SIZE))
#define chunkBlockIndexOOB(x, y, z) ((x) >= CHUNK_SIZE || (x) < 0 \
	|| (y) >= CHUNK_SIZE || (y) < 0 \
	|| (z) >= CHUNK_SIZE || (z) < 0)

#define chunkBlockIndexInBounds(x, y, z) ((x) < CHUNK_SIZE && (x) >= 0 \
	&& (y) < CHUNK_SIZE && (y) >= 0 \
	&& (z) < CHUNK_SIZE && (z) >= 0)

// Forward declaration
FWD_DECL typedef struct World World;
FWD_DECL typedef struct Chunk Chunk;

typedef struct LightAddNode {
    VECTOR position;
    Chunk* chunk;
} LightAddNode;

typedef struct LightRemoveNode {
    VECTOR position;
    Chunk* chunk;
    LightLevel light_value;
} LightRemoveNode;

typedef struct ChunkUpdates {
    HashMap* sunlight_add_queue;
    HashMap* sunlight_remove_queue;
    HashMap* light_add_queue;
    HashMap* light_remove_queue;
} ChunkUpdates;

typedef struct ChunkGenerationContext {
    u8 sunlight_heightmap[CHUNK_SIZE * CHUNK_SIZE];
} ChunkGenerationContext;

typedef struct Chunk {
    World* world;
    bool is_top: 1;
    bool lightmap_updated: 1;
    bool mesh_updated: 1;
    u8 _pad: 5;
    ChunkVisibility visibility;
    VECTOR position;
    ChunkMesh mesh;
    IBlock* blocks[CHUNK_DATA_SIZE];
    LightMap lightmap;
    ChunkUpdates updates;
    cvector(DroppedIItem) dropped_items;
} Chunk;

#endif // _PSXMC__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_
