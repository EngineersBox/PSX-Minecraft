#pragma once

#ifndef _PSX_MINECRAFT__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_
#define _PSX_MINECRAFT__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_

#include "../../../structure/cvector.h"
#include "chunk_mesh.h"
#include "../position.h"
#include "../../blocks/blocks.h"

#define CHUNK_SIZE 8
#define CHUNK_BLOCK_SIZE (CHUNK_SIZE * BLOCK_SIZE)
#define CHUNK_DATA_SIZE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_DIRECTIONS 3
#define CHUNK_AXIS_NEIGHBOURS (CHUNK_DIRECTIONS * 2)

#define chunkBlockIndex(x, y, z) ((z) + ((y) * CHUNK_SIZE) + ((x) * CHUNK_SIZE * CHUNK_SIZE))

// Forward declaration
typedef struct World World;

typedef struct {
    World* world;
    VECTOR position;
    ChunkMesh mesh;
    IBlock* blocks[CHUNK_DATA_SIZE];
    cvector(IItem*) dropped_items;
} Chunk;

#endif // _PSX_MINECRAFT__GAME_WORLD_CHUNK__CHUNK_STRUCTURE_H_