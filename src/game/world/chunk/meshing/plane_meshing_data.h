#pragma once

#ifndef _PSXMC__GAME_WORLD_CHUNK_MESHING__PLANE_MESHING_DATA_H_
#define _PSXMC__GAME_WORLD_CHUNK_MESHING__PLANE_MESHING_DATA_H_

#include "../chunk_structure.h"
#include "../../../lighting/lightmap.h"
#include "../../../util/inttypes.h"

#if defined(CHUNK_SIZE) && CHUNK_SIZE > 0 && CHUNK_SIZE <= 32 && isPowerOf2(CHUNK_SIZE)
    #define planeType(size, name) typedef GLUE(u, size) name[size]
    planeType(CHUNK_SIZE,BinaryMeshPlane);
#undef planeType
#else
#error CHUNK_SIZE must be in the interval (0, 32] and be a power of 2
#endif

typedef struct {
    const u8 face;
    const u8 axis;
    const u16 light_level_colour_scalar;
    const Block* block;
} PlaneMeshingDataKey;

typedef struct {
    PlaneMeshingDataKey key;
    BinaryMeshPlane plane;
} PlaneMeshingData;

int plane_meshing_data_compare(const void* a, const void* b, void* ignored);
u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1);

#endif // _PSXMC__GAME_WORLD_CHUNK_MESHING__PLANE_MESHING_DATA_H_
