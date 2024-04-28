#include "binary_greedy_mesher.h"

#include <psxgte.h>
#include "../../../util/interface99_extensions.h"
#include "../../../util/bits.h"
#include "../../../structure/hashmap.h"

// Forward declarations
typedef struct World World;
IBlock* worldGetBlock(const World* world, const VECTOR* position);
IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);

#define CHUNK_SIZE_P (CHUNK_SIZE + 2)

typedef struct {
    u8 axis;
    BlockID blockId;
    u32 y;
} PlaneMeshingDataKey;

typedef struct {
    PlaneMeshingDataKey key;
    BinaryMeshPlane value;
} PlaneMeshingData;

int plane_meshing_data_compare(const void* a, const void* b, void* udata) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    const int axis = cmp(pa->key.axis, pb->key.axis);
    const int blockId = cmp(pa->key.blockId, pb->key.blockId);
    const int y = cmp(pa->key.y, pb->key.y);
    if (axis != 0) {
        return axis;
    } else if (blockId != 0) {
        return blockId;
    }
    return y;
};

u32 plane_meshing_data_hash(const void* item, u32 seed0, u32 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_sip(&data->key, sizeof(data->key), seed0, seed1);
}

void binaryGreedyMesherBuildMesh(Chunk* chunk) {
    u32 axis_cols[3][CHUNK_SIZE_P][CHUNK_SIZE_P] = {0};
    u32 col_face_masks[6][CHUNK_SIZE_P][CHUNK_SIZE_P] = {0};
#define addVoxelToAxisCols(_iblock, x, y, z) \
    const IBlock* iblock = (_iblock); \
    if ((iblock) == NULL) { \
        continue; \
    } \
    const Block* block = VCAST_PTR(Block*, (iblock)); \
    if ((block)->type != BLOCKTYPE_EMPTY) { \
        __typeof__(x) _x = (x); \
        __typeof__(y) _y = (y); \
        __typeof__(z) _z = (z); \
        axis_cols[0][_z][_x] |= 1 << _y; \
        axis_cols[1][_y][_z] |= 1 << _x; \
        axis_cols[2][_y][_z] |= 1 << _z; \
    }
    // Inner chunk blocks
    for (u32 x = 0; x < CHUNK_SIZE; x++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
            for (u32 y = 0; y < CHUNK_SIZE; y++) {
                addVoxelToAxisCols(
                    chunk->blocks[chunkBlockIndex(x, y, z)],
                    x + 1,
                    y + 1,
                    z + 1
                );
            }
        }
    }
    // Neighbouring chunk blocks

    // Z
    for (u32 x = 0; x < CHUNK_SIZE_P; x++) {
        for (u32 z = 0; z < CHUNK_SIZE_P - 1; z++) {
            for (u32 y = 0; y < CHUNK_SIZE_P; y++) {
                const VECTOR position = vector_const_sub(
                    ((VECTOR) {
                        .vx = x,
                        .vy = y,
                        .vz = z,
                    }),
                    1
                );
                addVoxelToAxisCols(
                    worldGetBlock(chunk->world, &position),
                    x,
                    y,
                    z
                );
            }
        }
    }
    // Y
    for (u32 x = 0; x < CHUNK_SIZE_P; x++) {
        for (u32 z = 0; z < CHUNK_SIZE_P; z++) {
            for (u32 y = 0; y < CHUNK_SIZE_P - 1; y++) {
                const VECTOR position = vector_const_sub(
                    ((VECTOR) {
                        .vx = x,
                        .vy = y,
                        .vz = z,
                    }),
                    1
                );
                addVoxelToAxisCols(
                    worldGetBlock(chunk->world, &position),
                    x,
                    y,
                    z
                );
            }
        }
    }
    // X
    for (u32 x = 0; x < CHUNK_SIZE_P - 1; x++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
            for (u32 y = 0; y < CHUNK_SIZE; y++) {
                const VECTOR position = vector_const_sub(
                    ((VECTOR) {
                        .vx = x,
                        .vy = y,
                        .vz = z,
                    }),
                    1
                );
                addVoxelToAxisCols(
                    worldGetBlock(chunk->world, &position),
                    x,
                    y,
                    z
                );
            }
        }
    }
    // Face culling
    for (u32 axis = 0; 0 < 3; axis++) {
        for (u32 z = 0; z < CHUNK_SIZE_P; z++) {
            for (u32 x = 0; x < CHUNK_SIZE_P; x++) {
                const u32 col = axis_cols[axis][z][x];
                // Sample descending axis, set bit to true when air meets solid
                col_face_masks[2 * axis + 0][z][x] = col & !(col << 1);
                // Sample ascending axis, set bit to true when air meets solid
                col_face_masks[2 * axis + 1][z][x] = col & !(col >> 1);
            }
        }
    }
    typedef struct {
        void* key;
        BinaryMeshPlane value;
    } BGMElement;
    // BinaryMeshPlane data[6][BLOCK_COUNT][32] = {0};
    struct hashmap* data = hashmap_new(
        sizeof(BGMElement),
        0, 0, 0,
        plane_meshing_data_hash,
        plane_meshing_data_compare,
        free,
        NULL
    );
    // Find faces and build binary planes based on block types
    for (u32 axis = 0; axis < 6; axis++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
            for (u32 x = 0; x < CHUNK_SIZE; x++) {
                // Skip padding
                u32 col = col_face_masks[axis][z + 1][x + 1];
                // Remove right most padding value, always invalid
                col >>= 1;
                // Remote left most padding value, always invalid
                col &= !(1 << CHUNK_SIZE);
                while (col != 0) {
                    u8 y = trailing_zeros(col);
                    // Clear least significant bit set
                    col &= col - 1;
                    VECTOR voxel_pos;
                    switch (axis) {
                        case 0:
                        case 1:
                            // Down, up
                            voxel_pos = (VECTOR) {
                                .vx = x,
                                .vy = y,
                                .vz = z,
                            };
                            break;
                        case 2:
                        case 3:
                            // Left, Right
                            voxel_pos = (VECTOR) {
                                .vx = y,
                                .vy = z,
                                .vz = x,
                            };
                            break;
                        default:
                            // Front, Back
                            voxel_pos = (VECTOR) {
                                .vx = x,
                                .vy = z,
                                .vz = y,
                            };
                            break;
                    }
                    const ChunkBlockPosition chunk_block_position = (ChunkBlockPosition) {
                        .chunk = chunk->position,
                        .block = voxel_pos
                    };
                    const IBlock* current_block = worldGetChunkBlock(chunk->world, &chunk_block_position);
                    if (current_block == NULL) {
                        printf("[BINARY GREEDY MESHER] Null block returned constructing mask\n");
                        abort();
                        continue;
                    }
                    const Block* block = VCAST_PTR(Block*, current_block);
                    const PlaneMeshingData query = (PlaneMeshingData) {
                        .key = (PlaneMeshingDataKey) {
                            .axis = axis,
                            .blockId = block->id,
                            .y = y
                        },
                        .value = (BinaryMeshPlane) {0}
                    };
                    PlaneMeshingData* current = (PlaneMeshingData*) hashmap_get(data, &query);
                    if (current == NULL) {
                        current = malloc(sizeof(PlaneMeshingData));
                        current->key = query.key;
                        current->value = {0};
                        hashmap_set(data, current);
                    }
                    current->value[x] |= 1 << z;
                }
            }
        }
    }
    ChunkMesh* mesh = &chunk->mesh;
    size_t iter = 0;
    void* item;
    while(hashmap_iter(data, &iter, &item)) {
        const PlaneMeshingData* elem = item;
        binaryGreedyMesherConstructPlane(
            mesh,
            (FaceDirection) elem->key.axis,
            elem->key.y,
            elem->value
        );
    }
    hashmap_free(data);
}

void binaryGreedyMesherConstructPlane(ChunkMesh* mesh, FaceDirection face_dir, u32 axis, BinaryMeshPlane plane) {

}
