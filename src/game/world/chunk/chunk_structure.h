#pragma once

#ifndef _PSXMC__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_
#define _PSXMC__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_

#include "../../../structure/cvector.h"
#include "chunk_mesh.h"
#include "../position.h"
#include "../../blocks/blocks.h"

#define CHUNK_SIZE 8
#define CHUNK_BLOCK_SIZE (CHUNK_SIZE * BLOCK_SIZE)
#define CHUNK_DATA_SIZE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_DIRECTIONS 3
#define CHUNK_AXIS_NEIGHBOURS (CHUNK_DIRECTIONS * 2)
#define CHUNK_LIGHT_MAP_SIZE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

#define chunkBlockIndex(x, y, z) ((z) + ((y) * CHUNK_SIZE) + ((x) * CHUNK_SIZE * CHUNK_SIZE))

// Forward declaration
typedef struct World World;
typedef struct Chunk Chunk;
typedef u8 LightMap[CHUNK_LIGHT_MAP_SIZE];

typedef struct LightAddNode {
    VECTOR position;
    Chunk* chunk;
} LightAddNode;

typedef struct LightRemoveNode {
    VECTOR position;
    Chunk* chunk;
    u8 light_value;
} LightRemoveNode;

typedef struct ChunkUpdates {
    cvector(LightAddNode) sunlight_queue;
    cvector(LightAddNode) light_add_queue;
    cvector(LightRemoveNode) light_remove_queue;
} ChunkUpdates;

typedef struct Chunk {
    World* world;
    bool is_top;
    VECTOR position;
    ChunkMesh mesh;
    IBlock* blocks[CHUNK_DATA_SIZE];
    LightMap lightmap;
    ChunkUpdates updates;
    cvector(IItem*) dropped_items;
} Chunk;

#endif // _PSXMC__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_
