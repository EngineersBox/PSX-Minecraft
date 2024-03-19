#pragma once

#ifndef PSX_MINECRAFT_CHUNK_H
#define PSX_MINECRAFT_CHUNK_H

#include <psxgte.h>
#include <stdbool.h>

#include "../../../structure/cvector.h"
#include "chunk_mesh.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "../../blocks/blocks.h"
#include "../position.h"
#include "../../items/item.h"
#include "../../entity/player.h"

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

void chunkInit(Chunk* chunk);
void chunkDestroy(const Chunk* chunk);

void chunkGenerate2DHeightMap(Chunk* chunk, const VECTOR* position);
void chunkGenerate3DHeightMap(Chunk* chunk, const VECTOR* position);

void chunkGenerateMesh(Chunk* chunk);
void chunkClearMesh(Chunk* chunk);

bool chunkModifyVoxel(Chunk* chunk, const VECTOR* position, IBlock* block, IItem** item_result);

void chunkRender(Chunk* chunk, RenderContext* ctx, Transforms* transforms);

IBlock* chunkGetBlock(const Chunk* chunk, int x, int y, int z);
IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position);

void chunkUpdate(Chunk* chunk, Player* player);

#endif // PSX_MINECRAFT_CHUNK_H
