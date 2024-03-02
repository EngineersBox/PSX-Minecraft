#include "chunk.h"

#include <clip.h>
#include <inline_c.h>
#include <stdbool.h>
#include <smd/smd.h>

#include "../../structure/cvector.h"
#include "../../structure/primitive/primitive.h"
#include "../generation/noise.h"
#include "../../math/math_utils.h"
#include "../../util/interface99_extensions.h"

// Forward declaration
Block* worldGetBlock(const World* world, const VECTOR* position);

typedef struct {
    Block* block;
    int8_t normal;
} Mask;

bool compareMask(Mask m1, Mask m2) {
    if (m1.block == NULL && m2.block == NULL) {
        return true;
    }
    if (m1.block == NULL || m2.block == NULL) {
        return false;
    }
    return m1.block->id == m2.block->id && m1.normal == m2.normal;
}

// #define compareMask(m1, m2) (\
//     (((m1).block == NULL && (m2).block == NULL) || (m1).block->id == (m2).block->id)\
//     && (m1).normal == (m2).normal\
// )

void chunkInit(Chunk* chunk) {
    printf("[CHUNK: %d,%d,%d] Initialising mesh\n", inlineVec(chunk->position));
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
    printf("[CHUNK: %d,%d,%d] Generating 2D height map\n", inlineVec(chunk->position));
    chunkGenerate2DHeightMap(chunk, &chunk->position);
}

void chunkDestroy(const Chunk* chunk) {
    chunkMeshDestroy(&chunk->mesh);
}

void chunkGenerate2DHeightMap(Chunk* chunk, const VECTOR* position) {
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
            const int height = noise2d(xPos * ONE, zPos * ONE) >> 1; // Div by 2
            // clamp(
            //     noise,
            //     0,
            //     CHUNK_SIZE
            // );
            // printf("[CHUNK: %d,%d,%d] Height: %d\n", inlineVec(chunk->position), height);
            for (int32_t y = CHUNK_SIZE; y > 0; y--) {
                const int32_t worldY = (position->vy * CHUNK_SIZE) + (CHUNK_SIZE - y)
                                       + (CHUNK_SIZE * 6); // !IMPORTANT: TESTING OFFSET
                if (worldY < height - 3) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = VCAST(Block*, STONE_BLOCK_SINGLETON);
                    // printf("[CHUNK: %d,%d,%d] Stone @ %d,%d,%d\n", inlineVec(chunk->position), x, y - 1, z);
                } else if (worldY < height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = VCAST(Block*, DIRT_BLOCK_SINGLETON);
                    // printf("[CHUNK: %d,%d,%d] Dirt @ %d,%d,%d\n", inlineVec(chunk->position), x, y - 1, z);
                } else if (worldY == height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = VCAST(Block*, GRASS_BLOCK_SINGLETON);
                    // printf("[CHUNK: %d,%d,%d] Grass @ %d,%d,%d\n", inlineVec(chunk->position), x, y - 1, z);
                } else {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = VCAST(Block*, AIR_BLOCK_SINGLETON);
                    // printf("[CHUNK: %d,%d,%d] Air @ %d,%d,%d\n", inlineVec(chunk->position), x, y - 1, z);
                }
            }
        }
    }
}

void chunkGenerate3DHeightMap(Chunk* chunk, const VECTOR* position) {
    for (int32_t x = 0; x < CHUNK_SIZE; x++) {
        for (int32_t y = 0; y < CHUNK_SIZE; y++) {
            for (int32_t z = 0; z < CHUNK_SIZE; z++) {
                const int height = noise3d(
                    x + position->vx,
                    y + position->vy,
                    z + position->vz
                );
                printf("Height: %d\n", height);
                chunk->blocks[chunkBlockIndex(x, y, z)] = VCAST(
                    Block*,
                    height >= 0 ? AIR_BLOCK_SINGLETON : STONE_BLOCK_SINGLETON
                );
            }
        }
    }
}

void chunkClearMesh(Chunk* chunk) {
    chunkMeshClear(&chunk->mesh);
}

const INDEX INDICES[6] = {
    {0, 2, 1, 3},
    {2, 0, 3, 1},
    {1, 0, 3, 2},
    {0, 1, 2, 3},
    {1, 0, 3, 2},
    {0, 1, 2, 3},
};

SMD_PRIM* createQuadPrimitive(ChunkMesh* mesh,
                              const int width,
                              const int height,
                              const Mask* mask,
                              const int16_t axisMask[CHUNK_DIRECTIONS],
                              const int index) {
    // Construct a new POLY_FT4 (textured quad) primtive for this face
    // printf("Primitive %d\n", smd->n_prims);
    SMD_PRIM* p_prims = (SMD_PRIM*) mesh->p_prims;
    cvector_push_back(p_prims, (SMD_PRIM) {});
    // The SMD.p_prims field has been cast to an lvalue of SMD_PRIM*
    // to a separate variable. Any realloc that occurs will set the
    // new address to the local variable, we should propagate that
    // change to the SMD field.
    mesh->p_prims = p_prims;
    SMD_PRIM* primitive = &cvector_begin(p_prims)[mesh->n_prims++];
    primitive->prim_id = (SMD_PRI_TYPE){};
    primitive->prim_id.type = SMD_PRI_TYPE_QUAD;
    primitive->prim_id.l_type = SMD_PRI_TYPE_LIGHTING_FLAT;
    primitive->prim_id.c_type = SMD_PRI_TYPE_COLORING_GOURAUD;
    primitive->prim_id.texture = 1;
    primitive->prim_id.blend = 0; // TODO: Check this
    primitive->prim_id.zoff = 0; // TODO: Check this
    primitive->prim_id.nocull = 0;
    primitive->prim_id.mask = 0;
    primitive->prim_id.texwin = 0;
    primitive->prim_id.texoff = 0;
    primitive->prim_id.reserved = 0;
    primitive->prim_id.len = 4 + 8 + 4 + 8 + 4; // Some wizardry based on PSn00bSDK/tools/smxlink/main.cpp lines 518-644
    const Texture* texture = &textures[TERRAIN_TEXTURES];
    primitive->tpage = texture->tpage;
    primitive->clut = texture->clut;
    const TextureAttributes* attributes = &mask->block->faceAttributes[index];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = BLOCK_TEXTURE_SIZE * (axisMask[0] != 0 ? height : width);
    primitive->tv1 = BLOCK_TEXTURE_SIZE * (axisMask[0] != 0 ? width : height);
    primitive->r0 = attributes->tint.r;
    primitive->g0 = attributes->tint.g;
    primitive->b0 = attributes->tint.b;
    primitive->code = attributes->tint.cd;
    return primitive;
}

#define nextRenderAttribute(attribute_field, index_field, count_field, instance) \
        cvector_push_back(mesh->attribute_field, (SVECTOR){}); \
        primitive->index_field = mesh->count_field; \
        instance = &cvector_begin(mesh->attribute_field)[mesh->count_field++]

void createQuadVertices(Chunk* chunk,
                        const int16_t origin[CHUNK_DIRECTIONS],
                        const int16_t delta_axis_1[CHUNK_DIRECTIONS],
                        const int16_t delta_axis_2[CHUNK_DIRECTIONS],
                        SMD_PRIM* primitive,
                        const int index) {
    ChunkMesh* mesh = &chunk->mesh;
    // Construct vertices relative to chunk mesh bottom left origin
    const int16_t chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    // Offset by 1 to ensure bottom block of bottom chunk starts at Y = 0
    const int16_t chunk_origin_y = (-chunk->position.vy - 1) * CHUNK_SIZE;
    const int16_t chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
    const SVECTOR vertices[4] = {
        {
            (chunk_origin_x + origin[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2]) * BLOCK_SIZE
        },
        {
            (chunk_origin_x + origin[0] + delta_axis_1[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1] + delta_axis_1[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2] + delta_axis_1[2]) * BLOCK_SIZE
        },
        {
            (chunk_origin_x + origin[0] + delta_axis_2[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1] + delta_axis_2[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2] + delta_axis_2[2]) * BLOCK_SIZE
        },
        {
            (chunk_origin_x + origin[0] + delta_axis_1[0] + delta_axis_2[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1] + delta_axis_1[1] + delta_axis_2[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2] + delta_axis_1[2] + delta_axis_2[2]) * BLOCK_SIZE
        }
    };
    const INDEX indices = INDICES[index];
    SVECTOR* vertex = NULL;
    const SVECTOR* currentVert;
    #define bindVertex(v) nextRenderAttribute(p_verts, v, n_verts, vertex); \
        currentVert = &vertices[indices.v]; \
        vertex->vx = currentVert->vx; \
        vertex->vy = currentVert->vy; \
        vertex->vz = currentVert->vz
    bindVertex(v0);
    bindVertex(v1);
    bindVertex(v2);
    bindVertex(v3);
}

void createQuadNormal(ChunkMesh* mesh,
                      SMD_PRIM* primitive,
                      const Mask* mask,
                      const int16_t axisMask[CHUNK_DIRECTIONS]) {
    // Create normal for this quad
    SVECTOR* norm = NULL;
    nextRenderAttribute(p_norms, n0, n_norms, norm);
    norm->vx = (axisMask[0] * mask->normal) * ONE;
    norm->vy = (axisMask[1] * mask->normal) * ONE;
    norm->vz = (axisMask[2] * mask->normal) * ONE;
}

void createQuad(Chunk* chunk,
                const Mask* mask,
                const int16_t width,
                const int16_t height,
                const int16_t axisMask[CHUNK_DIRECTIONS],
                const int16_t origin[CHUNK_DIRECTIONS],
                const int16_t delta_axis_1[CHUNK_DIRECTIONS],
                const int16_t delta_axis_2[CHUNK_DIRECTIONS]) {
    // Calculate face index
    const int8_t shiftedNormal = (mask->normal + 2) / 2; // {-1,1} => {0, 1}
    const int index = (axisMask[0] * (1 - shiftedNormal)) // 0: -X, 1: +X
                      + (axisMask[1] * (3 - shiftedNormal)) // 2: -Y, 3: +Y
                      + (axisMask[2] * (5 - shiftedNormal)); // 4: -Z, 5: +Z
    ChunkMesh* mesh = &chunk->mesh;
    SMD_PRIM* primitive = createQuadPrimitive(
        mesh,
        width,
        height,
        mask,
        axisMask,
        index
    );
    createQuadVertices(
        chunk,
        origin,
        delta_axis_1,
        delta_axis_2,
        primitive,
        index
    );
    createQuadNormal(
        mesh,
        primitive,
        mask,
        axisMask
    );
}

void computeMeshMask(const Chunk* chunk,
                     const int axis,
                     const int axis1,
                     const int axis2,
                     int16_t chunkIter[CHUNK_DIRECTIONS],
                     int16_t axisMask[CHUNK_DIRECTIONS],
                     Mask mask[CHUNK_SIZE * CHUNK_SIZE]) {
    VECTOR query_position = {};
    uint16_t n = 0;
    for (chunkIter[axis2] = 0; chunkIter[axis2] < CHUNK_SIZE; chunkIter[axis2]++) {
        for (chunkIter[axis1] = 0; chunkIter[axis1] < CHUNK_SIZE; chunkIter[axis1]++) {
            query_position.vx = chunkIter[0] + (chunk->position.vx * CHUNK_SIZE);
            query_position.vy = chunkIter[1] + (chunk->position.vy * CHUNK_SIZE);
            query_position.vz = chunkIter[2] + (chunk->position.vz * CHUNK_SIZE);
            // BUG: The conditions on block retrieval causes extra mesh segments to
            //      be created at the top and bottom of each chunk regardless of the
            //      neighbouring chunk blocks. Without the condition there are still
            //      bottom segements of the mesh created amd also a ghost segement at
            //      the top of the world mirroring the bottom face of the bottom of
            //      the world.
            Block* currentBlock;
            if (0 <= chunkIter[axis]) {
                currentBlock = worldGetBlock(chunk->world, &query_position);
            } else {
                currentBlock = VCAST(Block*, AIR_BLOCK_SINGLETON);
            }
            if (currentBlock == NULL) {
                currentBlock = VCAST(Block*, AIR_BLOCK_SINGLETON);
            }
            // const BlockID currentBlock = worldGetBlock(chunk->world, &query_position);
            // TODO: Refactor to have a Block instance method for opacity check
            const bool currentOpaque = blockIsOpaque(currentBlock->id);
            query_position.vx += axisMask[0];
            query_position.vy += axisMask[1];
            query_position.vz += axisMask[2];
            Block* compareBlock;
            if (chunkIter[axis] < CHUNK_SIZE - 1) {
                compareBlock = worldGetBlock(chunk->world, &query_position);
            } else {
                compareBlock = VCAST(Block*, AIR_BLOCK_SINGLETON);
            }
            if (compareBlock == NULL) {
                compareBlock = VCAST(Block*, AIR_BLOCK_SINGLETON);
            }
            // const BlockID compareBlock = worldGetBlock(chunk->world, &query_position);
            const bool compareOpaque = blockIsOpaque(compareBlock->id);
            if (currentOpaque == compareOpaque) {
                mask[n++] = (Mask){VCAST(Block*, AIR_BLOCK_SINGLETON), 0};
            } else if (currentOpaque) {
                mask[n++] = (Mask){currentBlock, 1};
            } else {
                mask[n++] = (Mask){compareBlock, -1};
            }
        }
    }
}

void generateMeshLexicographically(Chunk* chunk,
                                   const int axis1,
                                   const int axis2,
                                   int16_t deltaAxis1[CHUNK_DIRECTIONS],
                                   int16_t deltaAxis2[CHUNK_DIRECTIONS],
                                   int16_t chunkIter[CHUNK_DIRECTIONS],
                                   int16_t axisMask[CHUNK_DIRECTIONS],
                                   Mask mask[CHUNK_SIZE * CHUNK_SIZE]) {
    uint16_t n = 0;
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
            for (width = 1; i + width < CHUNK_SIZE && compareMask(mask[n + width], currentMask); width++) {
            }
            // Compute the height of this quad and store it in h
            // This is done by checking if every block next to this row (range 0 to w) is also part of the mask.
            // For example, if w is 5 we currently have a quad of dimensions 1 x 5. To reduce triangle count,
            // greedy meshing will attempt to expand this quad out to CHUNK_SIZE x 5, but will stop if it reaches a hole in the mask
            bool done = false;
            int height;
            for (height = 1; j + height < CHUNK_SIZE; height++) {
                // Check each block next to this quad
                for (int w = 0; w < width; w++) {
                    // If there's a hole in the mask, exit
                    if (!compareMask(mask[n + w + (height * CHUNK_SIZE)], currentMask)) {
                        done = true;
                        break;
                    }
                }
                if (done) {
                    break;
                }
            }
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
            memset(deltaAxis1, 0, sizeof(int16_t) * CHUNK_DIRECTIONS);
            memset(deltaAxis2, 0, sizeof(int16_t) * CHUNK_DIRECTIONS);
            for (int h = 0; h < height; h++) {
                for (int w = 0; w < width; w++) {
                    mask[n + w + (h * CHUNK_SIZE)] = (Mask){
                        .block = VCAST(Block*, AIR_BLOCK_SINGLETON),
                        .normal = 0
                    };
                }
            }
            i += width;
            n += width;
        }
    }
}

void chunkGenerateMesh(Chunk* chunk) {
    // 0: X, 1: Y, 2: Z
    for (int axis = 0; axis < CHUNK_DIRECTIONS; axis++) {
        const int axis1 = (axis + 1) % CHUNK_DIRECTIONS;
        const int axis2 = (axis + 2) % CHUNK_DIRECTIONS;
        int16_t deltaAxis1[CHUNK_DIRECTIONS] = {0};
        int16_t deltaAxis2[CHUNK_DIRECTIONS] = {0};
        int16_t chunkIter[CHUNK_DIRECTIONS] = {0};
        int16_t axisMask[CHUNK_DIRECTIONS] = {0};
        axisMask[axis] = 1;
        Mask mask[CHUNK_SIZE * CHUNK_SIZE] = {0};
        for (chunkIter[axis] = -1; chunkIter[axis] < CHUNK_SIZE;) {
            // Compute mask
            computeMeshMask(
                chunk,
                axis,
                axis1,
                axis2,
                chunkIter,
                axisMask,
                mask
            );
            chunkIter[axis]++;
            generateMeshLexicographically(
                chunk,
                axis1,
                axis2,
                deltaAxis1,
                deltaAxis2,
                chunkIter,
                axisMask,
                mask
            );
        }
    }
}

void chunkRender(Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    static SVECTOR rotation = {0, 0, 0};
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

Block* chunkGetBlock(const Chunk* chunk, const int x, const int y, const int z) {
    if (checkIndexOOB(x, y, z)) return BLOCKID_AIR;
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

Block* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) return BLOCKID_AIR;
    return chunk->blocks[chunkBlockIndex(position->vx, position->vy, position->vz)];
}

bool chunkModifyVoxel(Chunk* chunk, const VECTOR* position, Block* block) {
    const int32_t x = position->vx;
    const int32_t y = positiveModulo(-position->vy - 1, CHUNK_SIZE);
    const int32_t z = position->vz;
    if (checkIndexOOB(x, y, z)) {
        return false;
    }
    chunk->blocks[chunkBlockIndex(x, y, z)] = block;
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
    return true;
}
