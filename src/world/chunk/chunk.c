#include "chunk.h"

#include <clip.h>
#include <inline_c.h>
#include <stdbool.h>
#include <smd/smd.h>

#include "../noise.h"
#include "../../util/math_utils.h"

#define inlineVec(vec) (vec).vx, (vec).vy, (vec).vz

void chunkInit(Chunk *chunk/*, const int seed*/) {
    printf("[CHUNK: %d,%d,%d] Initialising mesh\n", inlineVec(chunk->position));
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
    printf("[CHUNK: %d,%d,%d] Generating 2D height map\n", inlineVec(chunk->position));
    chunkGenerate2DHeightMap(chunk, &chunk->position);
    printf("[CHUNK: %d,%d,%d] Generating mesh\n", inlineVec(chunk->position));
    chunkGenerateMesh(chunk);
    // printf("[CHUNK: %d,%d,%d] Initialising SMD mesh data\n", inlineVec(chunk->position));
    // smdInitData(&chunk->mesh.smd);
    // chunk->noise = fnlCreateState();
    // chunk->noise.seed = seed;
}

void chunkDestroy(const Chunk *chunk) {
    chunkMeshDestroy(&chunk->mesh);
}

void chunkGenerate2DHeightMap(Chunk *chunk, const VECTOR *position) {
    for (int32_t x = 0; x < CHUNK_SIZE; x++) {
        for (int32_t z = 0; z < CHUNK_SIZE; z++) {
            const int32_t xPos = x + (position->vx * CHUNK_SIZE);
            const int32_t zPos = z + (position->vz * CHUNK_SIZE);
            // FMath::Clamp(
            //     FMath::RoundToInt(
            //         (Noise->GetNoise(Xpos, Ypos) + 1) * Size / 2
            //     ),
            //     0,
            //     Size
            // );
            const int height = noise2d(xPos * ONE, zPos * ONE);
            // clamp(
            //     noise,
            //     0,
            //     CHUNK_SIZE
            // );
            printf("height: %d\n", height);
            for (int32_t y = CHUNK_SIZE; y > 0; y--) {
                const int32_t worldY = (position->vy * CHUNK_SIZE) + (CHUNK_SIZE - y)
                    + (CHUNK_SIZE * 16); // !IMPORTANT: TESTING OFFSET
                // TODO: Change these chunkBlockIndex to be global lookups through world
                //       to ensure that blocks in next chunk boundary are visible to this chunk
                //       so the mesh is seamless
                if (worldY < height - 3) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) STONE;
                    printf("STONE @ %d,%d,%d\n", x, y - 1, z);
                } else if (worldY < height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) DIRT;
                    printf("DIRT @ %d,%d,%d\n", x, y - 1, z);
                } else if (worldY == height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) GRASS;
                    printf("GRASS @ %d,%d,%d\n", x, y - 1, z);
                } else {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) AIR;
                    printf("AIR @ %d,%d,%d\n", x, y - 1, z);
                }
            }
        }
    }
}

void chunkGenerate3DHeightMap(Chunk *chunk, const VECTOR *position) {
    for (int32_t x = 0; x < CHUNK_SIZE; x++) {
        for (int32_t y = 0; y < CHUNK_SIZE; y++) {
            for (int32_t z = 0; z < CHUNK_SIZE; z++) {
                const int height = noise3d(
                    x + position->vx,
                    y + position->vy,
                    z + position->vz
                );
                printf("Height: %d\n", height);
                chunk->blocks[chunkBlockIndex(x, y, z)] = (BlockID) (height >= 0 ? AIR : STONE);
            }
        }
    }
}

void chunkClearMesh(Chunk* chunk) {
    ChunkMesh* mesh = &chunk->mesh;
    chunkMeshClear(mesh);
    SMD* smd = &mesh->smd;
    smd->id[0] = 'S';
    smd->id[1] = 'M';
    smd->id[2] = 'D';
    smd->version = 0x01;
    smd->flags = 0x0;
    smd->p_verts = cvector_begin(mesh->vertices);
    smd->p_norms = cvector_begin(mesh->normals);
    smd->p_prims = cvector_begin(mesh->primitives);
}

typedef struct {
    int16_t block;
    int8_t normal;
} Mask;

#define CHUNK_DIRECTIONS 3
#define compareMask(m1, m2) ((m1).block == (m2).block && (m1).normal == (m2).normal)

// TODO: Break this into separate methods for each section
void createQuad(Chunk* chunk,
                const Mask* mask,
                const int16_t width,
                const int16_t height,
                const int16_t axisMask[CHUNK_DIRECTIONS],
                const int16_t origin[CHUNK_DIRECTIONS],
                const int16_t deltaAxis1[CHUNK_DIRECTIONS],
                const int16_t deltaAxis2[CHUNK_DIRECTIONS]) {
    ChunkMesh *mesh = &chunk->mesh;
    cvector_iterator(SMD_PRIM) primitiveIter = cvector_begin(mesh->primitives);
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->vertices);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->normals);
    SMD* smd = &mesh->smd;
    // Construct a new POLY_FT4 (textured quad) primtive for this face
    cvector_push_back(mesh->primitives, (SMD_PRIM) {});
    SMD_PRIM* primitive = &primitiveIter[smd->n_prims];
    smd->n_prims++;
    primitive->prim_id = (SMD_PRI_TYPE){};
    primitive->prim_id.type = PRIM_TYPE_QUAD;
    primitive->prim_id.l_type = PRIM_LIGHTING_FLAT;
    primitive->prim_id.c_type = PRIM_COLOURING_GOURAUD;
    primitive->prim_id.texture = 1;
    primitive->prim_id.blend = 0; // TODO: Check this
    primitive->prim_id.zoff = 0; // TODO: Check this
    primitive->prim_id.nocull = 0;
    primitive->prim_id.mask = 0;
    primitive->prim_id.texwin = 0;
    primitive->prim_id.texoff = 0;
    primitive->prim_id.reserved = 0;
    primitive->prim_id.len = 4 + 8 + 4 + 8 + 4; // Some wizardry based on PSn00bSDK/tools/smxlink/main.cpp lines 518-644
    // Construct vertices relative to chunk mesh top left origin
    SVECTOR* vertex = NULL;
    const int16_t chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    const int16_t chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
    const int16_t chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
    #define nextRenderAttribute(attribute_field, index_field, count_field, instance, iter) \
        cvector_push_back(mesh->attribute_field, (SVECTOR){}); \
        primitive->index_field = smd->count_field; \
        instance = &iter[smd->count_field++]
    if (mask->normal == -1) {
        nextRenderAttribute(vertices, v0, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2]) * BLOCK_SIZE;
        nextRenderAttribute(vertices, v1, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0] + deltaAxis1[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1] + deltaAxis1[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2] + deltaAxis1[2]) * BLOCK_SIZE;
        nextRenderAttribute(vertices, v2, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0] + deltaAxis2[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1] + deltaAxis2[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2] + deltaAxis2[2]) * BLOCK_SIZE;
        nextRenderAttribute(vertices, v3, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0] + deltaAxis1[0] + deltaAxis2[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1] + deltaAxis1[1] + deltaAxis2[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2] + deltaAxis1[2] + deltaAxis2[2]) * BLOCK_SIZE;
    } else {
        nextRenderAttribute(vertices, v0, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0] + deltaAxis1[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1] + deltaAxis1[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2] + deltaAxis1[2]) * BLOCK_SIZE;
        nextRenderAttribute(vertices, v1, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2]) * BLOCK_SIZE;
        nextRenderAttribute(vertices, v2, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0] + deltaAxis1[0] + deltaAxis2[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1] + deltaAxis1[1] + deltaAxis2[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2] + deltaAxis1[2] + deltaAxis2[2]) * BLOCK_SIZE;
        nextRenderAttribute(vertices, v3, n_verts, vertex, verticesIter);
        vertex->vx = (chunk_origin_x + origin[0] + deltaAxis2[0]) * BLOCK_SIZE;
        vertex->vy = (chunk_origin_y + origin[1] + deltaAxis2[1]) * BLOCK_SIZE;
        vertex->vz = (chunk_origin_z + origin[2] + deltaAxis2[2]) * BLOCK_SIZE;
    }
    // Create normal for this quad
    SVECTOR* norm = NULL;
    nextRenderAttribute(normals, n0, n_norms, norm, normalsIter);
    norm->vx = (axisMask[0] * mask->normal) * ONE; // !BUG: These produce out of bounds indexing on the 'norm' instance?
    norm->vy = (axisMask[1] * mask->normal) * ONE;
    norm->vz = (axisMask[2] * mask->normal) * ONE;
    const Texture* texture = &textures[BLOCK_TEXTURES];
    primitive->tpage = texture->tpage;
    primitive->clut = texture->clut;
    // TODO: Need to figure out how to get texture wrapping to work
    //       across a quad for a single 16x16 smapler of the given tpage + clut
    // Calculate face index
    const int8_t shiftedNormal = (mask->normal + 2) / 2; // {-1,1} => {0, 1}
    const int index = (axisMask[2] * (1 + shiftedNormal))  // 0: -Z, 1: +Z
                    + (axisMask[1] * (2 + shiftedNormal))  // 2: -Y, 3: +Y
                    + (axisMask[0] * (4 + shiftedNormal)); // 4: -X, 5: +X
    printf("Face index: %d\n", index);
    const TextureAttributes* attributes = &BLOCKS[mask->block].faceAttributes[index];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = 16 * width;
    primitive->tv1 = 16 * height;
    primitive->r0 = attributes->tint.r;
    primitive->g0 = attributes->tint.g;
    primitive->b0 = attributes->tint.b;
    primitive->code = attributes->tint.use;
    printf("Primtives: %d\n", smd->n_prims);
}

void chunkGenerateMesh(Chunk *chunk) {
    for (int axis = 0; axis < CHUNK_DIRECTIONS; axis++) {
        const int axis1 = (axis + 1) % CHUNK_DIRECTIONS;
        const int axis2 = (axis + 2) % CHUNK_DIRECTIONS;
        printf("Axis: [%d,%d,%d]\n", axis, axis1, axis2);
        int16_t deltaAxis1[CHUNK_DIRECTIONS] = {0};
        int16_t deltaAxis2[CHUNK_DIRECTIONS] = {0};
        int16_t chunkIter[CHUNK_DIRECTIONS] = {0};
        int16_t axisMask[CHUNK_DIRECTIONS] = {0};
        axisMask[axis] = 1;
        // TODO: Make this a uint8_t[CHUNK_SIZE] array where each bit is a bool, int type will need to change
        //       if CHUNK_SIZE is increased from 8 to 16 (uint8_t -> uint16_t)
        Mask mask[CHUNK_SIZE * CHUNK_SIZE] = {0};
        for (chunkIter[axis] = -1; chunkIter[axis] < CHUNK_SIZE;) {
            // Compute mask
            uint16_t n = 0;
            for (int ca2 = 0; ca2 < CHUNK_SIZE; ca2++) {
                chunkIter[axis2] = ca2;
                for (int ca1 = 0; ca1 < CHUNK_SIZE; ca1++) {
                    chunkIter[axis1] = ca1;
                    const BlockID currentBlock = chunkGetBlock(
                        chunk,
                        chunkIter[0],// + chunk->position.vx,
                        chunkIter[1],// + chunk->position.vy,
                        chunkIter[2]// + chunk->position.vz
                    );
                    const bool currentOpaque = BLOCKS[currentBlock].type != EMPTY;
                    const BlockID compareBlock = chunkGetBlock(
                        chunk,
                        chunkIter[0] + axisMask[0],// + chunk->position.vx,
                        chunkIter[1] + axisMask[1],// + chunk->position.vy,
                        chunkIter[2] + axisMask[2] // + chunk->position.vz
                    );
                    const bool compareOpaque = BLOCKS[compareBlock].type != EMPTY;
                    if (currentOpaque == compareOpaque) {
                        mask[n++] = (Mask){(uint16_t) NONE, 0};
                    } else if (currentOpaque) {
                        mask[n++] = (Mask){(int16_t) currentBlock, 1};
                    } else {
                        mask[n++] = (Mask){(int16_t) compareBlock, -1};
                    }
                }
            }
            chunkIter[axis]++;
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
                    int width;
                    for (width = 1; i + width < CHUNK_SIZE && compareMask(mask[n + width], currentMask); width++) {}
                    // Compute the height of this quad and store it in h
                    // This is done by checking if every block next to this row (range 0 to w) is also part of the mask.
                    // For example, if w is 5 we currently have a quad of dimensions 1 x 5. To reduce triangle count,
                    // greedy meshing will attempt to expand this quad out to CHUNK_SIZE x 5, but will stop if it reaches a hole in the mask
                    bool done = false;
                    int height;
                    for (height = 1; j + height < CHUNK_SIZE; height++) {
                        // Check each block next to this quad
                        for (int k = 0; k < width; ++k) {
                            // If there's a hole in the mask, exit
                            if (!compareMask(mask[n + k + (height * CHUNK_SIZE)], currentMask)) {
                                done = true;
                                break;
                            }
                        }
                        if (done) {
                            break;
                        }
                    }
                    deltaAxis1[0] = 0; deltaAxis1[1] = 0; deltaAxis1[2] = 0;
                    deltaAxis2[0] = 0; deltaAxis2[1] = 0; deltaAxis2[2] = 0;
                    deltaAxis1[axis1] = width;
                    deltaAxis2[axis2] = height;
                    createQuad(
                        chunk,
                        &currentMask,
                        width,
                        height,
                        axisMask,
                        chunkIter,
                        deltaAxis1,
                        deltaAxis2
                    );
                    for (int l = 0; l < height; l++) {
                        for (int k = 0; k < width; k++) {
                            mask[n + k + l * CHUNK_SIZE] = (Mask){(uint16_t) NONE, 0};
                        }
                    }
                    i += width,
                    n += width;
                }
            }
        }
    }
}

void chunkRender(Chunk *chunk, DisplayContext *ctx, Transforms *transforms) {
    SVECTOR rotation = {0, 0, 0};
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&rotation, &omtx);
    TransMatrix(&omtx, &chunk->position);
    // Multiply light matrix to object matrix
    MulMatrix0(&transforms->lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&transforms->geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    // Sort + render mesh
    chunkMeshRender(&chunk->mesh, ctx, transforms);
    // Restore matrix
    PopMatrix();
}

#define checkIndexOOB(x, y, z) ((x) >= CHUNK_SIZE || (x) < 0 \
	|| (y) >= CHUNK_SIZE || (y) < 0 \
	|| (z) >= CHUNK_SIZE || (z) < 0)

BlockID chunkGetBlock(const Chunk *chunk, const int x, const int y, const int z) {
    if (checkIndexOOB(x, y, z)) return AIR;
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

BlockID chunkGetBlockVec(const Chunk *chunk, const VECTOR *position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) return AIR;
    return chunk->blocks[chunkBlockIndex(position->vx, position->vy, position->vz)];
}

void chunkModifyVoxel(Chunk *chunk, const VECTOR *position, const EBlockID block) {
    const int32_t x = position->vx;
    const int32_t y = position->vy;
    const int32_t z = position->vz;
    if (checkIndexOOB(x, y, z)) return;
    chunk->blocks[chunkBlockIndex(x, y, z)] = (BlockID) block;
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
}
