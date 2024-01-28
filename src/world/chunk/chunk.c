#include "chunk.h"

#include <clip.h>
#include <inline_c.h>
#include <stdbool.h>
#include <smd/smd.h>

#include "../../primitive/primitive.h"
#include "../noise.h"
#include "../../util/math_utils.h"

// Forward declaration
BlockID worldGetBlock(const World* world, const VECTOR* position);

void chunkInit(Chunk* chunk/*, const int seed*/) {
    printf("[CHUNK: %d,%d,%d] Initialising mesh\n", inlineVec(chunk->position));
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
    printf("[CHUNK: %d,%d,%d] Generating 2D height map\n", inlineVec(chunk->position));
    chunkGenerate2DHeightMap(chunk, &chunk->position);
    // chunk->noise.seed = seed;
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
            const int height = noise2d(xPos * ONE, zPos * ONE);
            // clamp(
            //     noise,
            //     0,
            //     CHUNK_SIZE
            // );
            printf("height: %d\n", height);
            for (int32_t y = CHUNK_SIZE; y > 0; y--) {
                const int32_t worldY = (position->vy * CHUNK_SIZE) + (CHUNK_SIZE - y)
                                       + (CHUNK_SIZE * 13); // !IMPORTANT: TESTING OFFSET
                if (worldY < height - 3) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) BLOCKID_STONE;
                    // printf("STONE @ %d,%d,%d\n", x, y - 1, z);
                } else if (worldY < height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) BLOCKID_DIRT;
                    // printf("DIRT @ %d,%d,%d\n", x, y - 1, z);
                } else if (worldY == height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) BLOCKID_GRASS;
                    // printf("GRASS @ %d,%d,%d\n", x, y - 1, z);
                } else {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = (BlockID) BLOCKID_AIR;
                    // printf("AIR @ %d,%d,%d\n", x, y - 1, z);
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
                chunk->blocks[chunkBlockIndex(x, y, z)] = (BlockID) (height >= 0 ? BLOCKID_AIR : BLOCKID_STONE);
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
    smd->n_verts = 0;
    smd->n_norms = 0;
    smd->n_prims = 0;
}

typedef struct {
    int16_t block;
    int8_t normal;
} Mask;

#define compareMask(m1, m2) ((m1).block == (m2).block && (m1).normal == (m2).normal)

const INDEX INDICES[6] = {
    {0, 2, 1, 3},
    {2, 0, 3, 1},

    // {1, 0, 3, 2},
    // {0, 1, 2, 3},
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
    SMD* smd = &mesh->smd;
    // Construct a new POLY_FT4 (textured quad) primtive for this face
    // printf("Primitive %d\n", smd->n_prims);
    cvector_push_back(mesh->primitives, (SMD_PRIM) {});
    SMD_PRIM* primitive = &cvector_begin(mesh->primitives)[smd->n_prims];
    smd->n_prims++;
    primitive->prim_id = (SMD_PRI_TYPE){};
    primitive->prim_id.type = PRIMITIVE_TYPE_QUAD;
    primitive->prim_id.l_type = PRIMITIVE_LIGHTING_FLAT;
    primitive->prim_id.c_type = PRIMITIVE_COLOURING_GOURAUD;
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
    const TextureAttributes* attributes = &BLOCKS[mask->block].faceAttributes[index];
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
        primitive->index_field = smd->count_field; \
        instance = &cvector_begin(mesh->attribute_field)[smd->count_field++]

void createQuadVertices(Chunk* chunk,
                        const int16_t origin[CHUNK_DIRECTIONS],
                        const int16_t delta_axis_1[CHUNK_DIRECTIONS],
                        const int16_t delta_axis_2[CHUNK_DIRECTIONS],
                        SMD_PRIM* primitive,
                        const int index) {
    ChunkMesh* mesh = &chunk->mesh;
    SMD* smd = &chunk->mesh.smd;
    // Construct vertices relative to chunk mesh top left origin
    const int16_t chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    const int16_t chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
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
    nextRenderAttribute(vertices, v0, n_verts, vertex);
    const SVECTOR* currentVert = &vertices[indices.v0];
    vertex->vx = currentVert->vx;
    vertex->vy = currentVert->vy;
    vertex->vz = currentVert->vz;
    // printf("V0 @ %p: {%d,%d,%d}\n", vertex, vertex->vx, vertex->vy, vertex->vz);
    nextRenderAttribute(vertices, v1, n_verts, vertex);
    currentVert = &vertices[indices.v1];
    vertex->vx = currentVert->vx;
    vertex->vy = currentVert->vy;
    vertex->vz = currentVert->vz;
    // printf("V1 @ %p: {%d,%d,%d}\n", vertex, vertex->vx, vertex->vy, vertex->vz);
    nextRenderAttribute(vertices, v2, n_verts, vertex);
    currentVert = &vertices[indices.v2];
    vertex->vx = currentVert->vx;
    vertex->vy = currentVert->vy;
    vertex->vz = currentVert->vz;
    // printf("V2 @ %p: {%d,%d,%d}\n", vertex, vertex->vx, vertex->vy, vertex->vz);
    nextRenderAttribute(vertices, v3, n_verts, vertex);
    currentVert = &vertices[indices.v3];
    vertex->vx = currentVert->vx;
    vertex->vy = currentVert->vy;
    vertex->vz = currentVert->vz;
    // printf("V3 @ %p: {%d,%d,%d}\n", vertex, vertex->vx, vertex->vy, vertex->vz);
}

void createQuadNormal(ChunkMesh* mesh,
                      SMD_PRIM* primitive,
                      const Mask* mask,
                      const int16_t axisMask[CHUNK_DIRECTIONS]) {
    SMD* smd = &mesh->smd;
    // Create normal for this quad
    SVECTOR* norm = NULL;
    nextRenderAttribute(normals, n0, n_norms, norm);
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
            const BlockID currentBlock = worldGetBlock(chunk->world, &query_position);
            const bool currentOpaque = blockIsOpaque(currentBlock);
            query_position.vx += axisMask[0];
            query_position.vy += axisMask[1];
            query_position.vz += axisMask[2];
            const BlockID compareBlock = worldGetBlock(chunk->world, &query_position);
            const bool compareOpaque = blockIsOpaque(compareBlock);
            // printf(
            //     "(%d,%d,%d) %d == (%d,%d,%d) %d\n",
            //     chunkIter[0] + (chunk->position.vx * CHUNK_SIZE),
            //     chunkIter[1] + (chunk->position.vy * CHUNK_SIZE),
            //     chunkIter[2] + (chunk->position.vz * CHUNK_SIZE),
            //     currentOpaque,
            //     axisMask[0] + chunkIter[0] + (chunk->position.vx * CHUNK_SIZE),
            //     axisMask[1] + chunkIter[1] + (chunk->position.vy * CHUNK_SIZE),
            //     axisMask[2] + chunkIter[2] + (chunk->position.vz * CHUNK_SIZE),
            //     compareOpaque
            // );
            if (currentOpaque == compareOpaque) {
                mask[n++] = (Mask){(uint16_t) BLOCKID_NONE, 0};
            } else if (currentOpaque) {
                mask[n++] = (Mask){(int16_t) currentBlock, 1};
            } else {
                mask[n++] = (Mask){(int16_t) compareBlock, -1};
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
                        .block = (uint16_t) BLOCKID_NONE,
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

void chunkRender(Chunk* chunk, DisplayContext* ctx, Transforms* transforms) {
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

BlockID chunkGetBlock(const Chunk* chunk, const int x, const int y, const int z) {
    if (checkIndexOOB(x, y, z)) return BLOCKID_AIR;
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

BlockID chunkGetBlockVec(const Chunk* chunk, const VECTOR* position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) return BLOCKID_AIR;
    return chunk->blocks[chunkBlockIndex(position->vx, position->vy, position->vz)];
}

void chunkModifyVoxel(Chunk* chunk, const VECTOR* position, const EBlockID block) {
    const int32_t x = position->vx;
    const int32_t y = position->vy;
    const int32_t z = position->vz;
    if (checkIndexOOB(x, y, z)) return;
    chunk->blocks[chunkBlockIndex(x, y, z)] = (BlockID) block;
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
}
