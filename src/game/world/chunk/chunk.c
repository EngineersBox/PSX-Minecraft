#include "chunk.h"

#include <clip.h>
#include <cvector_utils.h>
#include <inline_c.h>
#include <stdbool.h>
#include <smd/smd.h>

#include "../../structure/cvector.h"
#include "../../structure/primitive/primitive.h"
#include "../generation/noise.h"
#include "../../math/math_utils.h"
#include "../../util/interface99_extensions.h"
#include "../../resources/asset_indices.h"

// Forward declaration
IBlock* worldGetBlock(const World* world, const VECTOR* position);

typedef struct {
    IBlock* block;
    int8_t normal;
} Mask;

// bool compareMask2(Mask m1, Mask m2) {
//     if (m1.block == NULL && m2.block == NULL) {
//         return true;
//     }
//     if (m1.block == NULL || m2.block == NULL) {
//         return false;
//     }
//     const Block* block1 = VCAST(Block*, *m1.block);
//     const Block* block2 = VCAST(Block*, *m2.block);
//     return block1->id == block2->id && m1.normal == m2.normal;
// }

#define compareMask(m1, m2) (\
    (m1.block == NULL && m2.block == NULL) \
    || ( \
        m1.block != NULL \
        && m2.block != NULL \
        && VCAST(Block*, *m1.block)->id == VCAST(Block*, *m2.block)->id \
        && m1.normal == m2.normal \
    ) \
)

void chunkInit(Chunk* chunk) {
    chunk->dropped_items = NULL;
    cvector_init(chunk->dropped_items, 0, NULL);
    printf("[CHUNK: %d,%d,%d] Initialising mesh\n", inlineVec(chunk->position));
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
    printf("[CHUNK: %d,%d,%d] Generating 2D height map\n", inlineVec(chunk->position));
    chunkGenerate2DHeightMap(chunk, &chunk->position);
}

void chunkDestroy(const Chunk* chunk) {
    chunkMeshDestroy(&chunk->mesh);
    cvector_clear(chunk->dropped_items);
    free(chunk->dropped_items);
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
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = stoneBlockCreate();
                    // printf(
                    //     "[CHUNK: %d,%d,%d] Stone %p @ %d,%d,%d\n",
                    //     inlineVec(chunk->position),
                    //     chunk->blocks[chunkBlockIndex(x, y - 1, z)],
                    //     x, y - 1, z
                    // );
                } else if (worldY < height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = dirtBlockCreate();
                    // printf(
                    //     "[CHUNK: %d,%d,%d] Dirt %p @ %d,%d,%d\n",
                    //     inlineVec(chunk->position),
                    //     chunk->blocks[chunkBlockIndex(x, y - 1, z)],
                    //     x, y - 1, z
                    // );
                } else if (worldY == height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = grassBlockCreate();
                    // printf(
                    //     "[CHUNK: %d,%d,%d] Grass %p @ %d,%d,%d\n",
                    //     inlineVec(chunk->position),
                    //     chunk->blocks[chunkBlockIndex(x, y - 1, z)],
                    //     x, y - 1, z
                    // );
                } else {
                    chunk->blocks[chunkBlockIndex(x, y - 1, z)] = airBlockCreate();
                    // printf(
                    //     "[CHUNK: %d,%d,%d] Air %p @ %d,%d,%d\n",
                    //     inlineVec(chunk->position),
                    //     chunk->blocks[chunkBlockIndex(x, y - 1, z)],
                    //     x, y - 1, z
                    // );
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
                chunk->blocks[chunkBlockIndex(x, y, z)] = height >= 0
                    ? airBlockCreate()
                    : stoneBlockCreate();
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
    const Texture* texture = &textures[ASSET_TEXTURES_TERRAIN_INDEX];
    primitive->tpage = texture->tpage;
    primitive->clut = texture->clut;
    const Block* block = VCAST(Block*, *mask->block);
    const TextureAttributes* attributes = &block->face_attributes[index];
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
        [0] = {
            (chunk_origin_x + origin[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2]) * BLOCK_SIZE
        },
        [1] = {
            (chunk_origin_x + origin[0] + delta_axis_1[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1] + delta_axis_1[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2] + delta_axis_1[2]) * BLOCK_SIZE
        },
        [2] = {
            (chunk_origin_x + origin[0] + delta_axis_2[0]) * BLOCK_SIZE,
            (chunk_origin_y + origin[1] + delta_axis_2[1]) * BLOCK_SIZE,
            (chunk_origin_z + origin[2] + delta_axis_2[2]) * BLOCK_SIZE
        },
        [3] = {
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

Mask createMask(IBlock* currentIBlock, IBlock* compareIBlock) {
    const Block* currentBlock = VCAST(Block*, *currentIBlock);
    const Block* compareBlock = VCAST(Block*, *compareIBlock);
    if (currentBlock->id == BLOCKID_AIR && compareBlock->id == BLOCKID_AIR) {
        return (Mask) { airBlockCreate(), 0 };
    }
    const bool currentOpaque = VCALL(*currentIBlock, isOpaque);
    const bool compareOpaque = VCALL(*compareIBlock, isOpaque);
    if (currentOpaque && !compareOpaque) {
        // Current block is visible
        return (Mask) { currentIBlock, 1 };
    }
    if (!currentOpaque && compareOpaque) {
        // Comparing block is visible
        return (Mask) { compareIBlock, -1 };
    }
    if (currentOpaque) {
        // Both blocks are opaque
        return (Mask) { airBlockCreate(), 0 };
    }
    // Both blocks are transparent
    // NOTE: Temp use of GRASS to mimic GLASS with transparency
    // if (currentBlock->id == BLOCKID_GRASS && compareBlock->id == BLOCKID_GRASS) {
    //     // Glass should not replicate faces between blocks
    //     return (Mask) { airBlockCreate(), 0 };
    // }
    // Both transparent but different types
    return (Mask) {
        currentBlock->id != BLOCKID_AIR ? currentIBlock : compareIBlock,
        currentBlock->id != BLOCKID_AIR ? 1 : -1
    };

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
            IBlock* currentBlock;
            if (0 <= chunkIter[axis]) {
                currentBlock = worldGetBlock(chunk->world, &query_position);
            } else {
                currentBlock = airBlockCreate();
            }
            if (currentBlock == NULL) {
                currentBlock = airBlockCreate();
            }
            // Block* curBlock = VCAST(Block*, *currentBlock);
            // const BlockID currentBlock = worldGetBlock(chunk->world, &query_position);
            // const bool currentOpaque = VCALL(*currentBlock, isOpaque);
            query_position.vx += axisMask[0];
            query_position.vy += axisMask[1];
            query_position.vz += axisMask[2];
            IBlock* compareBlock;
            if (chunkIter[axis] < CHUNK_SIZE - 1) {
                compareBlock = worldGetBlock(chunk->world, &query_position);
            } else {
                compareBlock = airBlockCreate();
            }
            if (compareBlock == NULL) {
                compareBlock = airBlockCreate();
            }
            // Block* comBlock = VCAST(Block*, *compareBlock);
            // const BlockID compareBlock = worldGetBlock(chunk->world, &query_position);
            // const bool compareOpaque = VCALL(*compareBlock, isOpaque);
            mask[n++] = createMask(currentBlock, compareBlock);
            // if (currentOpaque == compareOpaque) {
            //     // if (curBlock->id != BLOCKID_AIR) {
            //     //     mask[n++] = (Mask){ currentBlock, 1 };
            //     // } else if (comBlock->id != BLOCKID_AIR) {
            //     //     mask[n++] = (Mask){ compareBlock, -1 };
            //     // } else {
            //     //     mask[n++] = (Mask){ airBlockCreate(), 0 };
            //     // }
            //     mask[n++] = (Mask){ airBlockCreate(), 0 };
            // } else if (currentOpaque || curBlock->id != BLOCKID_AIR) {
            //     mask[n++] = (Mask){ currentBlock, 1 };
            // } else {
            //     mask[n++] = (Mask){ compareBlock, -1 };
            // }
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
                        .block = airBlockCreate(),
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

void chunkRenderDroppedItems(Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    IItem** item;
    cvector_for_each_in(item, chunk->dropped_items) {
        if (*item == NULL) {
            continue;
        }
        VCALL_SUPER(**item, Renderable, renderWorld, ctx, transforms);
    }
}

void chunkRender(Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    static SVECTOR rotation = {0, 0, 0};
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&rotation, &omtx);
    ApplyMatrixLV(&omtx, &transforms->translation_position, &transforms->translation_position);
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
    chunkRenderDroppedItems(chunk, ctx, transforms);
}

#define checkIndexOOB(x, y, z) ((x) >= CHUNK_SIZE || (x) < 0 \
	|| (y) >= CHUNK_SIZE || (y) < 0 \
	|| (z) >= CHUNK_SIZE || (z) < 0)

IBlock* chunkGetBlock(const Chunk* chunk, const int x, const int y, const int z) {
    if (checkIndexOOB(x, y, z)) {
        return airBlockCreate();
    }
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return airBlockCreate();
    }
    return chunk->blocks[chunkBlockIndex(position->vx, position->vy, position->vz)];
}

void constructItemPosition(const Chunk* chunk, const VECTOR* block_position, VECTOR* item_position) {
    // Construct vertices relative to chunk mesh bottom left origin
    const int16_t chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    // Offset by 1 to ensure bottom block of bottom chunk starts at Y = 0
    const int16_t chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
    const int16_t chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
    item_position->vx = (chunk_origin_x + block_position->vx) * BLOCK_SIZE + (BLOCK_SIZE / 2);
    item_position->vy = (chunk_origin_y - block_position->vy) * BLOCK_SIZE - (BLOCK_SIZE / 2);
    item_position->vz = (chunk_origin_z + block_position->vz) * BLOCK_SIZE + (BLOCK_SIZE / 2);
}

bool chunkModifyVoxel(Chunk* chunk, const VECTOR* position, IBlock* block, IItem** item_result) {
    const int32_t x = position->vx;
    const int32_t y = positiveModulo(-position->vy - 1, CHUNK_SIZE);
    const int32_t z = position->vz;
    if (checkIndexOOB(x, y, z)) {
        return false;
    }
    const IBlock* old_block = chunk->blocks[chunkBlockIndex(x, y, z)];
    cvector_push_back(
        chunk->dropped_items,
        NULL
    );
    IItem* iitem = chunk->dropped_items[cvector_size(chunk->dropped_items) - 1] = VCALL(*old_block, destroy);
    if (iitem != NULL && iitem->self != NULL) {
        Item* item = VCAST(Item*, *iitem);
        constructItemPosition(chunk, position, &item->position);
        if (item_result != NULL) {
            *item_result = iitem;
        }
    } else {
        cvector_erase(chunk->dropped_items, cvector_size(chunk->dropped_items) - 1);
        if (item_result != NULL) {
            *item_result = NULL;;
        }
    }
    chunk->blocks[chunkBlockIndex(x, y, z)] = block;
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
    return true;
}

// This only works because PS1 games are single threaded (mostly)
Inventory* _current_inventory = NULL;

bool itemPickupValidator(const Item* item) {
    // 1. Does the item already exist in the inventory?
    //   a. [1:TRUE] Does the existing have space?
    //     i. [a:TRUE] Return true
    //     ii. [a:FALSE] Go to 2
    //   b. [1:FALSE] Go to 2
    // 2. Is there space in the inventory
    //   a. [2:TRUE] Return true
    //   b. [2:FALSE] Return false
    uint8_t from_slot = INVENTORY_SLOT_STORAGE_OFFSET;
    uint8_t next_free = INVENTORY_NO_FREE_SLOT;
    while (1) {
        const Slot* slot = inventorySearchItem(_current_inventory, item->id, from_slot, &next_free);
        if (slot == NULL) {
            if (next_free == INVENTORY_NO_FREE_SLOT) {
                return false;
            }
            slot = &_current_inventory->slots[next_free];
            if (inventorySlotGetItem(slot) == NULL) {
                break;
            }
        }
        const Item* slot_item = VCAST(Item*, *inventorySlotGetItem(slot));
        const int stack_left = slot_item->max_stack_size - slot_item->stack_size;
        if (stack_left != 0) {
            break;
        }
        from_slot = slot->index + 1;
        next_free = INVENTORY_NO_FREE_SLOT;
    }
    return true;
}

void chunkUpdate(Chunk* chunk, Player* player) {
    _current_inventory = VCAST(Inventory*, player->inventory);
    const VECTOR pos = (VECTOR) {
        .vx = player->position.vx >> FIXED_POINT_SHIFT,
        .vy = player->position.vy >> FIXED_POINT_SHIFT,
        .vz = player->position.vz >> FIXED_POINT_SHIFT,
    };
    IItem** iitem;
    uint32_t i = 0;
    cvector_for_each_in(iitem, chunk->dropped_items) {
        if (*iitem == NULL) {
            continue;
        }
        Item* item = VCAST(Item*, **iitem);
        if (itemUpdate(item, &pos, itemPickupValidator)) {
            // BUG: Something causes invalid address read when picking up new blocks.
            //      Hotbar:
            //        - 0: x26 Grass
            //        - 1: x64 Grass
            //        - 2: x1 Stone
            //        - 3: x1 Dirt
            //      At this point another Stone or Dirt block got picked up then an
            //      invalid read occurred at 0x00000004 with result INVENTORY_STORE_RESULT_ADDED_ALL
            //      This could possible be an error when invoking VCALL(**iitem, destroy)
            //      or itemDestroy(*item). But that could be a red herring as it falls through
            //      to the cvector_erase(...) in the INVENTORY_STORE_RESULT_ADDED_NEW_SLOT
            //      case block.
            printf("[ITEM] Picked up: %s x%d\n", item->name, item->stack_size);
            const InventoryStoreResult result = inventoryStoreItem(_current_inventory, *iitem);
            printf("[ITEM] Result: %s\n", inventoryStoreResultStringify(result));
            switch (result) {
                case INVENTORY_STORE_RESULT_ADDED_SOME:
                    // Do nothing, already updated iitem that was picked up as dropped.
                case INVENTORY_STORE_RESULT_NO_SPACE:
                    // Do nothing since we can't pick it up (don't think this will ever
                    // actually occur since we already check in itemPickupValidator for
                    // this case when determining which to items to consider.
                    break;
                case INVENTORY_STORE_RESULT_ADDED_ALL:
                    // Nuke it, added all so this item instance is not needed any more.
                    // Break is not used here since we still need to erase this array
                    // entry.
                    VCALL(**iitem, destroy);
                    itemDestroy(*iitem);
                case INVENTORY_STORE_RESULT_ADDED_NEW_SLOT:
                    // We reuse this item instance as the inventory instance now so don't
                    // free it.
                    cvector_erase(chunk->dropped_items, i);
                    break;
            }
            // BUG: Something here causes an invalid instruction error
            //      in dynarec (probably bad pointer stuff)
            continue;
        }
        i++;
    }
}
