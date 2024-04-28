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
    const u8 axis;
    const Block* block;
    const u32 y;
} PlaneMeshingDataKey;

typedef struct {
    PlaneMeshingDataKey key;
    BinaryMeshPlane value;
} PlaneMeshingData;

int plane_meshing_data_compare(const void* a, const void* b, void* udata) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    const int axis = cmp(pa->key.axis, pb->key.axis);
    const int blockId = cmp(pa->key.block->id, pb->key.block->id);
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
#undef addVoxelToAxisCols
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
    // BinaryMeshPlane data[6][BLOCK_COUNT][32] = {0};
    struct hashmap* data = hashmap_new(
        sizeof(PlaneMeshingData),
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
                // Remove left most padding value, always invalid
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
                            .block = block,
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
    chunkMeshClear(mesh);
    size_t iter = 0;
    void* item;
    while(hashmap_iter(data, &iter, &item)) {
        const PlaneMeshingData* elem = item;
        binaryGreedyMesherConstructPlane(
            mesh,
            (FaceDirection) elem->key.axis,
            elem->key.y,
            elem->value,
            CHUNK_SIZE
        );
    }
    hashmap_free(data);
}

UNIMPLEMENTED void createQuad(ChunkMesh* mesh,
                const FaceDirection face_dir,
                const u32 axis,
                const u32 x,
                const u32 y,
                const u32 w,
                const u32 h) {
    // TODO: Finish this referencing original chunk meshing code as well as TanTan's stuff
}

void binaryGreedyMesherConstructPlane(ChunkMesh* mesh,
                                      const FaceDirection face_dir,
                                      const u32 axis,
                                      BinaryMeshPlane plane,
                                      const u32 lod_size) {
    for (u32 row = 0; row < CHUNK_SIZE; row++) {
        u32 y = 0;
        while (y < lod_size) {
            y += trailing_zeros(plane[row] >> y);
            if (y >= lod_size) {
                // At top
                continue;
            }
            const u32 h = trailing_ones(plane[row] >> y);
            // Convert height n to positive bits repeated n times:
            // 1 = 0b1, 2 = 0b11, 4 = 0b1111
            u32 h_as_mask;
            if (h < 32) {
                h_as_mask = (1 << h) - 1;
            } else {
                h_as_mask = UINT32_MAX;
            }
            const u32 mask = h_as_mask << y;
            // Grow horizontally
            u32 w = 1;
            while (row + w < lod_size) {
                // Fetch bits spanning height in the next row
                const u32 next_row_h = (plane[row + w] >> y) & h_as_mask;
                if (next_row_h != h_as_mask) {
                    // Cannot grow further horizontally
                    break;
                }
                // Unset the bits we expanded into
                plane[row + w] &= ~mask;
                w++;
            }
            createQuad(
                mesh,
                face_dir,
                axis,
                y,
                w,
                h,
                row
            );
        }
    }
}

VECTOR faceDirectionPosition(const FaceDirection face_dir, const i32 axis, const i32 x, const i32 y) {
    VECTOR position;
#define vec(_x,_y,_z) position = (VECTOR) { .vx = _x, .vy = _y, .vz = _z }
    switch (face_dir) {
        case FACE_DIR_UP: vec(x, axis + 1, y); break;
        case FACE_DIR_DOWN: vec(x, axis, y); break;
        case FACE_DIR_LEFT: vec(axis, y, x); break;
        case FACE_DIR_RIGHT: vec(axis + 1, y, x); break;
        case FACE_DIR_FORWARD: vec(x, y, axis); break;
        case FACE_DIR_BACK: vec(x, y, axis + 1); break;
    }
#undef vec
    return position;
}
