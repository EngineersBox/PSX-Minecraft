#pragma once

#ifndef PSX_MINECRAFT_CHUNK_MESH_H
#define PSX_MINECRAFT_CHUNK_MESH_H

#include <smd/smd.h>

#include "../../blocks/block.h"
#include "../../util/cvector.h"

typedef struct {
    BlockID block;
    uint8_t normal;
} BlockFace;

typedef struct {
    int width;
    int height;
} UV;

#define PRIM_TYPE_LINE	0
#define PRIM_TYPE_TRI	1
#define PRIM_TYPE_QUAD	2

#define PRIM_LIGHTING_NONE 0	// No shading (no normals)
#define PRIM_LIGHTING_FLAT 1	// Flat shading (1 normal)
#define PRIM_LIGHTING_SMOOTH 2	// Smooth shading (3 normals per vertex)

#define PRIM_COLOURING_SOLID 0
#define PRIM_COLOURING_GOURAUD 1

typedef struct {
    SMD smd;
    cvector(SMD_PRIM) primitives;
    cvector(SVECTOR) vertices;
    cvector(SVECTOR) normals;
} ChunkMesh;

void chunkMeshInit(ChunkMesh* mesh);
void chunkMeshDestroy(const ChunkMesh* mesh);
void chunkMeshClear(ChunkMesh* mesh);

#endif // PSX_MINECRAFT_CHUNK_MESH_H
