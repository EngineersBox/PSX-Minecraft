#pragma once

#ifndef PSX_MINECRAFT_CHUNK_MESH_H
#define PSX_MINECRAFT_CHUNK_MESH_H

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

typedef struct {
    cvector(SVECTOR) vertices;
    cvector(int) triangles;
    cvector(SVECTOR) normals;
    cvector(BlockFace) blockFaces;
    cvector(UV) uv;
} ChunkMesh;

#endif // PSX_MINECRAFT_CHUNK_MESH_H
