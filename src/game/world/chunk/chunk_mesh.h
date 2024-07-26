#pragma once

#ifndef PSXMC_CHUNK_MESH_H
#define PSXMC_CHUNK_MESH_H

#include <smd/smd.h>

#include "../../../render/render_context.h"
#include "../../../render/transforms.h"
#include "../../../structure/primitive/direction.h"
#include "../../../math/vector.h"
#include "../../../lighting/lightmap.h"

// TODO: Move this to global defines file
#define CHUNK_SIZE 8

#define MESH_PRIMITIVE_TYPE_COUNT 3
#define MESH_PRIMITIVE_TYPE_BIN_COUNT 2
typedef enum MeshPrimitiveType {
    MESH_PRIM_TYPE_LINE = 0,
    MESH_PRIM_TYPE_TRIANGLE,
    MESH_PRIM_TYPE_QUAD
} MeshPrimitiveType;

typedef struct MeshPrimitive {
    MeshPrimitiveType type: MESH_PRIMITIVE_TYPE_BIN_COUNT;
    u8 _pad: 8 - MESH_PRIMITIVE_TYPE_BIN_COUNT;
    u16 v0, v1, v2, v3; // Vertex indices
    u16 n0, n1, n2, n3; // Normal indices
    LightLevel light_level;
    CVECTOR colour;
    u8 r, g, b;
    u8 tu0, tv0;
    u8 tu1, tv1;
    u16 tpage, clut;
} MeshPrimitive;

typedef struct Mesh {
	u8  version;
	u16 flags;
	u16 n_verts;
	u16 n_norms;
	u16 n_prims;
	SVECTOR* p_verts;
	SVECTOR* p_norms;
	void* p_prims;
} Mesh;

typedef struct {
    Mesh face_meshes[FACE_DIRECTION_COUNT];
} ChunkMesh;

void chunkMeshInit(ChunkMesh* mesh);
void chunkMeshDestroy(const ChunkMesh* mesh);
void chunkMeshClear(ChunkMesh* mesh);

void chunkMeshRenderFaceDirection(const Mesh* mesh,
                                  const LightLevel internal_light_level,
                                  RenderContext* ctx,
                                  Transforms* transforms);
void chunkMeshRender(const ChunkMesh* mesh,
                     const LightLevel internal_light_level,
                     RenderContext* ctx,
                     Transforms* transforms);

#endif // PSXMC_CHUNK_MESH_H
