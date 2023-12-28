#include "chunk.h"

#include <stdbool.h>

#define CHUNK_DIRECTIONS 3

BlockID worldGetBlock(int x, int y, int z);

typedef struct {
    int16_t block;
    int8_t normal;
} Mask;

#define compareMask(m1, m2) ((m1).block == (m2).block && (m1).normal == (m2).normal)

#define vec3Mul(type, vec, scalar) ((type) { (vec).vx * (scalar), (vec).vy * (scalar), (vec).vz * (scalar) })

void createQuad(Chunk* chunk,
                const Mask* mask,
                const VECTOR axisMask,
                const int width,
                const int height,
                const VECTOR v1,
                const VECTOR v2,
                const VECTOR v3,
                const VECTOR v4) {
    const SVECTOR normal = (SVECTOR){
        axisMask.vx * mask->normal,
        axisMask.vy * mask->normal,
        axisMask.vz * mask->normal
    };
    const BlockFace face = (BlockFace){
        mask->block,
        mask->normal
    };
    ChunkMesh* mesh = &chunk->mesh;
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v1, BLOCK_SIZE));
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v2, BLOCK_SIZE));
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v3, BLOCK_SIZE));
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v4, BLOCK_SIZE));

    const int vertCount = chunk->vertexCount;
    cvector_push_back(mesh->triangles, vertCount);
    cvector_push_back(mesh->triangles, vertCount + 2 + mask->normal);
    cvector_push_back(mesh->triangles, vertCount + 2 - mask->normal);
    cvector_push_back(mesh->triangles, vertCount + 3);
    cvector_push_back(mesh->triangles, vertCount + 1 - mask->normal);
    cvector_push_back(mesh->triangles, vertCount + 1 + mask->normal);

    cvector_push_back(mesh->normals, normal);
    cvector_push_back(mesh->normals, normal);
    cvector_push_back(mesh->normals, normal);
    cvector_push_back(mesh->normals, normal);

    cvector_push_back(mesh->blockFaces, face);
    cvector_push_back(mesh->blockFaces, face);
    cvector_push_back(mesh->blockFaces, face);
    cvector_push_back(mesh->blockFaces, face);

    if (normal.vx == 1 || normal.vx == -1) {
        UV uv0 = {width, height};
        cvector_push_back(mesh->uv, uv0);
        UV uv1 = {0, height};
        cvector_push_back(mesh->uv, uv1);
        UV uv2 = {width, 0};
        cvector_push_back(mesh->uv, uv2);
        UV uv3 = {0, 0};
        cvector_push_back(mesh->uv, uv3);
    } else {
        UV uv0 = {height, width};
        cvector_push_back(mesh->uv, uv0);
        UV uv1 = {height, 0};
        cvector_push_back(mesh->uv, uv1);
        UV uv2 = {0, width};
        cvector_push_back(mesh->uv, uv2);
        UV uv3 = {0, 0};
        cvector_push_back(mesh->uv, uv3);
    }
    chunk->vertexCount += 4;
}

void chunkGenerateMesh(Chunk *chunk) {
    for (int d = 0; d < CHUNK_DIRECTIONS; d++) {
        int k, width, height;
        const int axis1 = (d + 1) % CHUNK_DIRECTIONS;
        const int axis2 = (d + 2) % CHUNK_DIRECTIONS;
        int deltaAxis1[CHUNK_DIRECTIONS] = {0};
        int deltaAxis2[CHUNK_DIRECTIONS] = {0};
        int chunkIter[CHUNK_DIRECTIONS] = {0};
        // TODO: Make this a uint8_t[CHUNK_SIZE] array where each bit is a bool
        Mask mask[CHUNK_SIZE * CHUNK_SIZE];
        for (chunkIter[d] = -1; chunkIter[d] < CHUNK_SIZE;) {
            int axisMask[CHUNK_DIRECTIONS] = {0};
            // Compute mask
            uint8_t n = 0;
            for (chunkIter[axis2] = 0; chunkIter[axis1] < CHUNK_SIZE; chunkIter[axis2]++) {
                for (chunkIter[axis1] = 0; chunkIter[axis1] < CHUNK_SIZE; chunkIter[axis1]++) {
                    const BlockID currentBlock = chunkGetBlock(
                        chunk,
                        chunkIter[0] + chunk->position.vx,
                        chunkIter[1] + chunk->position.vy,
                        chunkIter[2] + chunk->position.vz
                    );
                    const bool currentOpaque = BLOCKS[currentBlock].type == SOLID;
                    const BlockID compareBlock = chunkGetBlock(
                        chunk,
                        chunkIter[0] + axisMask[0] + chunk->position.vx,
                        chunkIter[1] + axisMask[1] + chunk->position.vy,
                        chunkIter[2] + axisMask[2] + chunk->position.vz
                    );
                    const bool compareOpaque = BLOCKS[compareBlock].type == SOLID;
                    if (currentOpaque == compareOpaque) {
                        mask[n++] = (Mask){-1, 0};
                    } else if (currentOpaque) {
                        mask[n++] = (Mask){(int16_t) currentBlock, 1};
                    } else {
                        mask[n++] = (Mask){(int16_t) compareBlock, -1};
                    }
                }
            }
            chunkIter[d]++;
            n = 0;
            // Generate a mesh from mask using lexicographic ordering
            // looping over each block in this slice of the chunk
            for (int j = 0; j < CHUNK_SIZE; j++) {
                for (int i = 0; i < CHUNK_SIZE;) {
                    if (mask[n].normal == 0) {
                        i++;
                        n++;
                        continue;
                    }
                    const Mask currentMask = mask[n];
                    chunkIter[axis1] = i;
                    chunkIter[axis2] = j;
                    // Compute the width of this quad and store it in w
                    // This is done by searching along the current axis until mask[n + w] is false
                    for (width = 1; i + width < CHUNK_SIZE && compareMask(mask[n + width], currentMask); width++) {}

                    // Compute the height of this quad and store it in h
                    // This is done by checking if every block next to this row (range 0 to w) is also part of the mask.
                    // For example, if w is 5 we currently have a quad of dimensions 1 x 5. To reduce triangle count,
                    // greedy meshing will attempt to expand this quad out to CHUNK_SIZE x 5, but will stop if it reaches a hole in the mask
                    bool done = false;
                    for (height = 1; j + height < CHUNK_SIZE; height++) {
                        // Check each block next to this quad
                        for (k = 0; k < width; ++k) {
                            // If there's a hole in the mask, exit
                            if (compareMask(mask[n + k + height * CHUNK_SIZE], currentMask)) continue;
                            done = true;
                            break;
                        }
                        if (done) break;
                    }
                    deltaAxis1[axis1] = width;
                    deltaAxis2[axis2] = height;
                    createQuad(
                        chunk,
                        &currentMask,
                        (VECTOR) {axisMask[0], axisMask[1], axisMask[2]},
                        width,
                        height,
                        (VECTOR) {chunkIter[0], chunkIter[1], chunkIter[2]},
                        (VECTOR){
                            chunkIter[0] + deltaAxis1[0],
                            chunkIter[1] + deltaAxis1[1],
                            chunkIter[2] + deltaAxis1[2]
                        },
                        (VECTOR){
                            chunkIter[0] + deltaAxis2[0],
                            chunkIter[1] + deltaAxis2[1],
                            chunkIter[2] + deltaAxis2[2]
                        },
                        (VECTOR){
                            chunkIter[0] + deltaAxis1[0] + deltaAxis2[0],
                            chunkIter[1] + deltaAxis1[1] + deltaAxis2[1],
                            chunkIter[2] + deltaAxis1[2] + deltaAxis2[2]
                        }
                    );
                    deltaAxis1[axis1] = 0;
                    deltaAxis2[axis2] = 0;
                    for (int l = 0; l < height; l++) {
                        for (k = 0; k < width; k++) {
                            mask[n + k + l * CHUNK_SIZE] = (Mask){0, 0};
                        }
                    }
                    i += width,
                    n += width;
                }
            }
        }
    }
}

void chunkRender(Chunk* chunk, DisplayContext* ctx, Transforms* transforms) {
    
}

#define checkIndexOOB(x, y, z) ((x) >= CHUNK_SIZE || (x) < 0 \
	|| (y) >= CHUNK_SIZE || (y) < 0 \
	|| (z) >= CHUNK_SIZE || (z) < 0)

BlockID chunkGetBlock(const Chunk *chunk, const int x, const int y, const int z) {
    if (checkIndexOOB(x, y, z)) return 0;
    return chunk->data[chunkBlockIndex(x, y, z)];
}

BlockID chunkGetBlockVec(const Chunk *chunk, const VECTOR *position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) return 0;
    return chunk->data[chunkBlockIndex(position->vx, position->vy, position->vz)];
}
