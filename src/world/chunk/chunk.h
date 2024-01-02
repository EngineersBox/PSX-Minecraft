#pragma once

#ifndef PSX_MINECRAFT_CHUNK_H
#define PSX_MINECRAFT_CHUNK_H

#include <psxgte.h>
#include "chunk_mesh.h"
#include "../../core/display.h"
#include "../../render/transforms.h"
#include "../../blocks/block.h"
#include "../position.h"
#include "../../util/fast_noise_lite.h"

// !BUG: Why does 16 not render anything but 8 does?
#define CHUNK_SIZE 8
#define CHUNK_DATA_SIZE (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

#define chunkBlockIndex(x, y, z) ((x) + ((y) * CHUNK_SIZE) + ((z) * CHUNK_SIZE * CHUNK_SIZE))

typedef struct {
    VECTOR position;
    ChunkMesh mesh;
    // fnl_state noise;
    BlockID blocks[CHUNK_DATA_SIZE];
} Chunk;

void chunkInit(Chunk* chunk/*, int seed*/);
void chunkDestroy(const Chunk* chunk);

void chunkGenerate2DHeightMap(Chunk* chunk, const VECTOR* position);
void chunkGenerate3DHeightMap(Chunk* chunk, const VECTOR* position);

void chunkGenerateMesh(Chunk* chunk);
void chunkClearMesh(Chunk* chunk);

void chunkModifyVoxel(Chunk* chunk, const VECTOR* position, EBlockID block);

void chunkRender(Chunk* chunk, DisplayContext* ctx, Transforms* transforms);

BlockID chunkGetBlock(const Chunk* chunk, int x, int y, int z);
BlockID chunkGetBlockVec(const Chunk* chunk, const VECTOR* position);

#endif // PSX_MINECRAFT_CHUNK_H
