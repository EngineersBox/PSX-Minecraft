#pragma once

#ifndef _GAME_WORLD_CHUNK_MESHING__MESHING_H_
#define _GAME_WORLD_CHUNK_MESHING__MESHING_H_

#include "plane_meshing_data.h"
#include "../chunk_structure.h"
#include "../chunk_mesh.h"

#define nextRenderAttribute(mesh, attribute_field, index_field, count_field) ({ \
        cvector_push_back((mesh)->attribute_field, (SVECTOR){0}); \
        primitive->index_field = (mesh)->count_field; \
        (&cvector_begin((mesh)->attribute_field)[(mesh)->count_field++]); \
})

extern const INDEX QUAD_INDICES[FACE_DIRECTION_COUNT];

typedef void (*MeshingFn)(Chunk* chunk,
                          const PlaneMeshingDataKey* data,
                          const Texture* texture_override,
                          const TextureAttributes* texture_attributes_override,
                          const u32 x,
                          const u32 y,
                          const u32 w,
                          const u32 h);

MeshPrimitive* createPrimitive(ChunkMesh* mesh,
                               const PlaneMeshingDataKey* data,
                               const u32 width,
                               const u32 height,
                               const Texture* texture_override,
                               const TextureAttributes texture_attributes_override[FACE_DIRECTION_COUNT]);

SVECTOR createVertex(const i32 chunk_origin_x,
                     const i32 chunk_origin_y,
                     const i32 chunk_origin_z,
                     const FaceDirection face_dir,
                     const i32 axis,
                     const fixedi32 axis_offset, // [0, BLOCK_SIZE)
                     const i32 x,
                     const i32 y);

void createVertices(Chunk* chunk,
                    MeshPrimitive* primitive,
                    const FaceDirection face_dir,
                    const i32 axis,
                    const fixedi32 axis_offset,
                    const i32 x,
                    const i32 y,
                    const i32 w,
                    const i32 h);

void createNormal(ChunkMesh* mesh,
                  MeshPrimitive* primitive,
                  const Block* block,
                  const FaceDirection face_dir);

#endif // _GAME_WORLD_CHUNK_MESHING__MESHING_H_
