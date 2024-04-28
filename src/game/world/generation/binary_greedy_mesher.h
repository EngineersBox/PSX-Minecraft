#pragma once

#ifndef _PSX_MINECRAFT__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_
#define _PSX_MINECRAFT__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_

#include "../chunk/chunk_structure.h"
#include "../chunk/chunk_mesh.h"
#include "../../../structure/cvector.h"
#include "../../../util/inttypes.h"
#include "../../../util/preprocessor.h"
#include "../../../math/math_utils.h"

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

void binaryGreedyMesherBuildMesh(Chunk* chunk);
UNIMPLEMENTED void binaryGreedyMesherConstructPlane(ChunkMesh* mesh, FaceDirection face_dir, u32 axis, BinaryMeshPlane plane);

#endif // _PSX_MINECRAFT__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_