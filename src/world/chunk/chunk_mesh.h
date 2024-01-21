#pragma once

#ifndef PSX_MINECRAFT_CHUNK_MESH_H
#define PSX_MINECRAFT_CHUNK_MESH_H

#include <smd/smd.h>

#include "../../util/cvector.h"
#include "../../core/display.h"
#include "../../render/transforms.h"

#ifndef MESH_PRIMITIVE_VEC_INITIAL_CAPCITY
#define MESH_PRIMITIVE_VEC_INITIAL_CAPCITY 1
#endif

#ifndef MESH_VERTEX_VEC_INITIAL_CAPCITY
#define MESH_VERTEX_VEC_INITIAL_CAPCITY 1
#endif

#ifndef MESH_NORMAL_VEC_INITIAL_CAPCITY
#define MESH_NORMAL_VEC_INITIAL_CAPCITY 1
#endif

// TODO: Move these to SMD renderer file as general properties
typedef enum {
    PRIMITIVE_TYPE_LINE = 0,
    PRIMITIVE_TYPE_TRIANGLE,
    PRIMITIVE_TYPE_QUAD,
} PrimitiveType;
#define PRIMITIVE_TYPE_LINE 0
#define PRIMITIVE_TYPE_TRIANGLE 1
#define PRIMITIVE_TYPE_QUAD 2

#define PRIMITIVE_LIGHTING_NONE 0 // No shading (no normals)
#define PRIMITIVE_LIGHTING_FLAT 1 // Flat shading (1 normal)
#define PRIMITIVE_LIGHTING_SMOOTH 2 // Smooth shading (3 normals per vertex)

#define PRIMITIVE_COLOURING_SOLID 0
#define PRIMITIVE_COLOURING_GOURAUD 1

// TODO: Refactor to just be SMD instead of wrapper object
//       Need to be aware that SMD.p_prims is a void* type
//       and not SMD_PRIM*. Sizes of elements will be different
//       need to cast before performing any operations.
typedef struct {
    SMD smd;
    cvector(SMD_PRIM) primitives;
    cvector(SVECTOR) vertices;
    cvector(SVECTOR) normals;
} ChunkMesh;

void chunkMeshInit(ChunkMesh* mesh);
void chunkMeshDestroy(const ChunkMesh* mesh);
void chunkMeshClear(ChunkMesh* mesh);
void chunkMeshRender(const ChunkMesh* mesh, DisplayContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_CHUNK_MESH_H
