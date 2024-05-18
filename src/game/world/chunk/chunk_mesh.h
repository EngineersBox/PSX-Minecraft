#pragma once

#ifndef PSX_MINECRAFT_CHUNK_MESH_H
#define PSX_MINECRAFT_CHUNK_MESH_H

#include <smd/smd.h>

#include "../../render/render_context.h"
#include "../../render/transforms.h"

#ifndef MESH_PRIMITIVE_VEC_INITIAL_CAPCITY
#define MESH_PRIMITIVE_VEC_INITIAL_CAPCITY 0
#endif

#ifndef MESH_VERTEX_VEC_INITIAL_CAPCITY
#define MESH_VERTEX_VEC_INITIAL_CAPCITY 0
#endif

#ifndef MESH_NORMAL_VEC_INITIAL_CAPCITY
#define MESH_NORMAL_VEC_INITIAL_CAPCITY 0
#endif

// typedef SMD ChunkMesh;

typedef struct {
    SMD face_meshes[6];
} ChunkMesh;

void chunkMeshInit(ChunkMesh* mesh);
void chunkMeshDestroy(const ChunkMesh* mesh);
void chunkMeshClear(ChunkMesh* mesh);

void chunkMeshRenderFaceDirection(const SMD* mesh, RenderContext* ctx, Transforms* transforms);
void chunkMeshRender(const ChunkMesh* mesh, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_CHUNK_MESH_H
