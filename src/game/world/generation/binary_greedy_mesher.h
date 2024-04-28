#pragma once

#ifndef _PSX_MINECRAFT__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_
#define _PSX_MINECRAFT__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_

#include "../chunk/chunk_structure.h"
#include "../chunk/chunk_mesh.h"
#include "../../../structure/cvector.h"
#include "../../../util/inttypes.h"
#include "../../../util/preprocessor.h"
#include "../../../math/math_utils.h"

// This implementation is based on TanTan's Binary Greedy Mesher demo:
// https://github.com/TanTanDev/binary_greedy_mesher_demo/blob/main/src/greedy_mesher_optimized.rs

#if defined(CHUNK_SIZE) && CHUNK_SIZE > 0 && CHUNK_SIZE <= 32 && isPowerOf2(CHUNK_SIZE)
    #define planeType(size, name) typedef GLUE(u, size) name[size]
    planeType(CHUNK_SIZE,BinaryMeshPlane);
    #undef planeType
#else
    #error CHUNK_SIZE must be in the interval (0, 32] and be a power of 2
#endif

typedef enum {
    FACE_DIR_DOWN = 0,
    FACE_DIR_UP,
    FACE_DIR_LEFT,
    FACE_DIR_RIGHT,
    FACE_DIR_FORWARD,
    FACE_DIR_BACK
} FaceDirection;

VECTOR faceDirectionPosition(const FaceDirection face_dir, const i32 axis, const i32 x, const i32 y);

void binaryGreedyMesherBuildMesh(Chunk* chunk);
// lod_size here should be a value in the interval [0,CHUNK_SIZE]
// where CHUNK_SIZE as a value indicates 1:1 with every block
void binaryGreedyMesherConstructPlane(ChunkMesh* mesh, const FaceDirection face_dir, const u32 axis, BinaryMeshPlane plane, const u32 lod_size);

#endif // _PSX_MINECRAFT__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_