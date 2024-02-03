#pragma once

#ifndef PSX_MINECRAFT_CHUNK_H
#define PSX_MINECRAFT_CHUNK_H

#include <psxgte.h>
#include "chunk_mesh.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "../../blocks/block.h"
#include "../position.h"
#include "../../util/fast_noise_lite.h"

// !BUG: Why does 16 not render anything but 8 does?
#define CHUNK_SIZE 8
#define CHUNK_BLOCK_SIZE (CHUNK_SIZE * BLOCK_SIZE)
#define CHUNK_DATA_SIZE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_DIRECTIONS 3
#define CHUNK_AXIS_NEIGHBOURS (CHUNK_DIRECTIONS * 2)

#define chunkBlockIndex(x, y, z) ((x) + ((y) * CHUNK_SIZE) + ((z) * CHUNK_SIZE * CHUNK_SIZE))

// Forward declaration
typedef struct World World;

typedef struct {
    World* world;
    VECTOR position;
    ChunkMesh mesh;
    BlockID blocks[CHUNK_DATA_SIZE];
} Chunk;

void chunkInit(Chunk* chunk);
void chunkDestroy(const Chunk* chunk);

void chunkGenerate2DHeightMap(Chunk* chunk, const VECTOR* position);
void chunkGenerate3DHeightMap(Chunk* chunk, const VECTOR* position);

void chunkGenerateMesh(Chunk* chunk);
void chunkClearMesh(Chunk* chunk);

void chunkModifyVoxel(Chunk* chunk, const VECTOR* position, EBlockID block);

void chunkRender(Chunk* chunk, RenderContext* ctx, Transforms* transforms);

BlockID chunkGetBlock(const Chunk* chunk, int x, int y, int z);
BlockID chunkGetBlockVec(const Chunk* chunk, const VECTOR* position);

#endif // PSX_MINECRAFT_CHUNK_H
