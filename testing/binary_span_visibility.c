#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef int8_t i8;

typedef uint16_t u16;
typedef int16_t i16;

typedef uint32_t u32;
typedef int32_t i32;

typedef uint64_t u64;
typedef int64_t i64;

typedef u8 fixedu8;
typedef i8 fixedi8;

typedef u16 fixedu16;
typedef i16 fixedi16;

typedef u32 fixedu32;
typedef i32 fixedi32;

typedef u64 fixedu64;
typedef i64 fixedi64;

typedef struct _MATRIX {
	int16_t m[3][3];
	int32_t t[3];
} MATRIX;

typedef struct _VECTOR {
	int32_t vx, vy, vz;
} VECTOR;

typedef struct _SVECTOR {
	int16_t vx, vy, vz, pad;
} SVECTOR;

typedef struct _CVECTOR {
	uint8_t r, g, b, cd;
} CVECTOR;

typedef struct _DVECTOR {
	int16_t vx, vy;
} DVECTOR;

#define CHUNK_SIZE 4

typedef bool Block;

typedef struct Chunk {
    // Y, Z, X
    Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} Chunk;

#define chunkGetBlock(chunk, x, y, z) &(chunk)->blocks[(CHUNK_SIZE * CHUNK_SIZE * (y)) + (CHUNK_SIZE * (z)) + (x)]

Block* worldGetBlock(Chunk* chunk, const VECTOR* pos) {
    if (pos->vx < 0 || pos->vy < 0 || pos->vz < 0) {
        return NULL;
    } else if (pos->vx >= CHUNK_SIZE || pos->vy >= CHUNK_SIZE || pos->vz >= CHUNK_SIZE) {
        return NULL;
    }
    return chunkGetBlock(chunk, pos->vx, pos->vy, pos->vz);
}

int main() {
    const Chunk chunk = (Chunk) {
        .blocks = {
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,

            1, 1, 1, 1,
            1, 0, 0, 1,
            1, 0, 0, 1,
            1, 1, 1, 1,

            0, 0, 0, 0,
            0, 1, 1, 0,
            0, 1, 1, 0,
            0, 0, 0, 0,

            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        },
    };
    return 0;
}

#define FACE_DIRECTION_COUNT 6
#define FACE_DIRECTION_COUNT_BITS 3
typedef enum FaceDirection {
    FACE_DIR_DOWN = 0,
    FACE_DIR_UP,
    FACE_DIR_LEFT,
    FACE_DIR_RIGHT,
    FACE_DIR_BACK,
    FACE_DIR_FRONT
} FaceDirection;

#define ONE (1 << 12)


#define INLINE __attribute__((always_inline)) inline
#define CHUNK_SIZE_PADDED (CHUNK_SIZE + 2)
#define AXIS_COUNT 3
#define AXIAL_EDGES_COUNT 2
static const u32 AXIAL_EDGES[AXIAL_EDGES_COUNT] = { 0, CHUNK_SIZE_PADDED - 1 };

typedef u32 FacesColumns[FACE_DIRECTION_COUNT][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED];

INLINE void addVoxelToFaceColumns(FacesColumns axis_cols,
                                  FacesColumns axis_cols_opaque,
                                  const Block* block,
                                  const u32 x,
                                  const u32 y,
                                  const u32 z) {
    if (block == NULL) {
        return;
    }
    if (!*block) {
        return;
    }
    axis_cols[FACE_DIR_DOWN][z][x] |= 1 << y;
    axis_cols[FACE_DIR_UP][z][x] |= 1 << y;
    axis_cols[FACE_DIR_LEFT][y][z] |= 1 << x;
    axis_cols[FACE_DIR_RIGHT][y][z] |= 1 << x;
    axis_cols[FACE_DIR_BACK][y][x] |= 1 << z;
    axis_cols[FACE_DIR_FRONT][y][x] |= 1 << z;
    const u8 bitset = 0b1111111;
#define bitsetAt(i) ((bitset >> (i)) & 0b1)
    axis_cols_opaque[FACE_DIR_DOWN][z][x] |= bitsetAt(1) << y;
    axis_cols_opaque[FACE_DIR_UP][z][x] |= bitsetAt(0) << y;
    axis_cols_opaque[FACE_DIR_LEFT][y][z] |= bitsetAt(3) << x;
    axis_cols_opaque[FACE_DIR_RIGHT][y][z] |= bitsetAt(2) << x;
    axis_cols_opaque[FACE_DIR_BACK][y][x] |= bitsetAt(4) << z;
    axis_cols_opaque[FACE_DIR_FRONT][y][x] |= bitsetAt(5) << z;
#undef bitsetAt
}

void binaryGreedyMesherBuildMesh(Chunk* chunk) {
    FacesColumns faces_cols = {0};
    // See binary_greedy_mesher_transparency.md
    FacesColumns faces_cols_opaque = {0};
    FacesColumns col_face_masks = {0};
    // Inner chunk blocks
    // Duplication here is just to optimise away lots of if statements
    // when this chunk has no breaking state available as a paramater
    for (u32 z = 0; z < CHUNK_SIZE; z++) {
        for (u32 x = 0; x < CHUNK_SIZE; x++) {
            for (u32 y = 0; y < CHUNK_SIZE; y++) {
                const Block* block = chunkGetBlock(chunk, x, y, z);
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    block,
                    x + 1,
                    y + 1,
                    z + 1
                );
            }
        }
    }
    // Neighbouring chunk blocks
    // Z
    for (u32 z_i = 0; z_i < AXIAL_EDGES_COUNT; z_i++) {
        for (i32 x = 0; x < CHUNK_SIZE_PADDED; x++) {
            for (i32 y = 0; y < CHUNK_SIZE_PADDED; y++) {
                const i32 z = AXIAL_EDGES[z_i];
                const VECTOR position = (VECTOR){
                    x - 1,
                    y - 1,
                    z - 1
                };
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    worldGetBlock(chunk, &position),
                    x,
                    y,
                    z
                );
            }
        }
    }
    // Y
    for (i32 z = 0; z < CHUNK_SIZE_PADDED; z++) {
        for (i32 x = 0; x < CHUNK_SIZE_PADDED; x++) {
            for (u32 y_i = 0; y_i < AXIAL_EDGES_COUNT; y_i++) {
                const i32 y = AXIAL_EDGES[y_i];
                const VECTOR position = (VECTOR){
                    x - 1,
                    y - 1,
                    z - 1
                };
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    worldGetBlock(chunk, &position),
                    x,
                    y,
                    z
                );
            }
        }
    }
    // X
    for (i32 z = 0; z < CHUNK_SIZE; z++) {
        for (u32 x_i = 0; x_i < AXIAL_EDGES_COUNT; x_i++) {
            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                const i32 x = AXIAL_EDGES[x_i];
                const VECTOR position = (VECTOR){
                    x - 1,
                    y - 1,
                    z - 1
                };
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    worldGetBlock(chunk, &position),
                    x,
                    y,
                    z
                );
            }
        }
    }
    // Face culling
    for (u32 axis = 0; axis < AXIS_COUNT; axis++) {
        for (u32 z = 0; z < CHUNK_SIZE_PADDED; z++) {
            for (u32 x = 0; x < CHUNK_SIZE_PADDED; x++) {
                // TODO: For chunks that are on the edge of the render distance
                //       we should not generate faces that face outward toward
                //       unloaded chunks. To do this we should ensure that the
                //       padding area in col and col_opaque are always set to 1's
                //       for the parts that face outwards.
                const u8 axis0 = (2 * axis) + 0;
                const u8 axis1 = (2 * axis) + 1;
                const u32 col_axis0 = faces_cols[axis0][z][x];
                const u32 col_axis1 = faces_cols[axis1][z][x];
                const u32 col_opaque_axis0 = col_axis0 & faces_cols_opaque[axis0][z][x];
                const u32 col_opaque_axis1 = col_axis1 & faces_cols_opaque[axis1][z][x];
                // Solid
                const u32 solid_descending = col_axis0 & ~(col_axis0 << 1);
                const u32 solid_ascending = col_axis1 & ~(col_axis1 >> 1);
                // Transparent
                const u32 opaque_descending = col_opaque_axis0 & ~(col_opaque_axis0 << 1);
                const u32 opaque_ascending = col_opaque_axis1 & ~(col_opaque_axis1 >> 1);
                // Combine to ensure any faces behind a transparent face are kept
                col_face_masks[axis0][z][x] = solid_descending | opaque_descending;
                col_face_masks[axis1][z][x] = solid_ascending | opaque_ascending;
            }
        }
    }
    HashMap* data = hashmap_new(
        sizeof(PlaneMeshingData),
        0,
        0,
        0,
        plane_meshing_data_hash,
        plane_meshing_data_compare,
        NULL,
        NULL
    );
    // Find faces and build binary planes based on block types
    for (u32 face = 0; face < FACE_DIRECTION_COUNT; face++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
            for (u32 x = 0; x < CHUNK_SIZE; x++) {
                // Skip padding
                u32 col = col_face_masks[face][z + 1][x + 1];
                // Remove right most padding value, always invalid
                col >>= 1;
                // Remove left most padding value, always invalid
                col &= ~(1 << CHUNK_SIZE);
                while (col != 0) {
                    const u8 y = trailing_zeros(col);
                    // Clear least significant bit set
                    col &= col - 1;
                    ChunkBlockPosition chunk_block_position = {
                        .block = vec3_i32_all(0),
                        .chunk = chunk->position
                    };
                    ChunkBlockPosition light_cb_pos = {
                        .block = vec3_i32_all(0),
                        .chunk = chunk->position
                    };
                    switch (face) {
                        case FACE_DIR_DOWN:
                            chunk_block_position.block = vec3_i32(x, y, z);
                            light_cb_pos.block = vec3_i32(x, y - 1, z);
                            break;
                        case FACE_DIR_UP:
                            chunk_block_position.block = vec3_i32(x, y, z);
                            light_cb_pos.block = vec3_i32(x, y + 1, z);
                            break;
                        case FACE_DIR_LEFT:
                            chunk_block_position.block = vec3_i32(y, z, x);
                            light_cb_pos.block = vec3_i32(y - 1, z, x);
                            break;
                        case FACE_DIR_RIGHT:
                            chunk_block_position.block = vec3_i32(y, z, x);
                            light_cb_pos.block = vec3_i32(y + 1, z, x);
                            break;
                        case FACE_DIR_BACK:
                            chunk_block_position.block = vec3_i32(x, z, y);
                            light_cb_pos.block = vec3_i32(x, z, y - 1);
                            break;
                        case FACE_DIR_FRONT:
                            chunk_block_position.block = vec3_i32(x, z, y);
                            light_cb_pos.block = vec3_i32(x, z, y + 1);
                            break;
                    }
                    const VECTOR world_block_position = chunkBlockToWorldPosition(
                        &chunk_block_position,
                        CHUNK_SIZE
                    );
                    if (breaking_state != NULL
                        && breaking_state->block != NULL
                        && vec3_equal(breaking_state->position, world_block_position)) {
                        // Don't generate faces that match with the breaking block
                        // as we will create those later with the overlay. This avoids
                        // z fighting between mesh faces and overlay faces.
                        continue;
                    }
                    IBlock* current_block = worldGetChunkBlock(
                        chunk->world,
                        &chunk_block_position
                    );
                    if (current_block == NULL) {
                        errorAbort(
                            "[BINARY GREEDY MESHER] Null block returned while constructing mask [Face: %d] [Chunk: " VEC_PATTERN "] [Block: " VEC_PATTERN "]\n",
                            face,
                            VEC_LAYOUT(chunk->position),
                            VEC_LAYOUT(chunk_block_position.block)
                        );
                        continue;
                    }
                    const Block* block = VCAST_PTR(Block*, current_block);
                    if (blockGetType(block->id) == BLOCKTYPE_EMPTY) {
                        errorAbort(
                            "[BINARY GREEDY MESHER] Empty block encountered while constructing mask [Face: %d] [Chunk: " VEC_PATTERN "] [Block: " VEC_PATTERN "]\n",
                            face,
                            VEC_LAYOUT(chunk->position),
                            VEC_LAYOUT(chunk_block_position.block)
                        );
                        continue;
                    }
                    // NOTE: For blocks that are transparent to sunlight, we should
                    //       look up the position of the block without the direction
                    //       offset applied.
                    const VECTOR light_query_pos = chunkBlockToWorldPosition(
                        !blockIsFaceOpaque(block, face)
                            ? &chunk_block_position
                            : &light_cb_pos,
                        CHUNK_SIZE
                    );
                    const LightLevel light_level = worldGetLightValue(
                        chunk->world,
                        &light_query_pos
                    );
                    const PlaneMeshingData query = (PlaneMeshingData) {
                        .key = (PlaneMeshingDataKey) {
                            face,
                            y,
                            light_level,
                            block
                        },
                        .plane = {0},
                    };
                    PlaneMeshingData* current = (PlaneMeshingData*) hashmap_get(data, &query);
                    if (current == NULL) {
                        hashmap_set(data, &query);
                        current = (PlaneMeshingData*) hashmap_get(data, &query);
                    }
                    current->plane[x] |= 1 << z;
                }
            }
        }
    }
    chunkMeshClear(&chunk->mesh);
    size_t iter = 0;
    void* item;
    while (hashmap_iter(data, &iter, &item)) {
        PlaneMeshingData* elem = item;
        binaryGreedyMesherConstructPlane(
            chunk,
            elem,
            CHUNK_SIZE
        );
    }
    hashmap_free(data);
}

void binaryGreedyMesherConstructPlane(Chunk* chunk,
                                      PlaneMeshingData* data,
                                      const u32 lod_size) {
    for (u32 row = 0; row < CHUNK_SIZE; row++) {
        u32 y = 0;
        while (y < lod_size) {
            y += trailing_zeros(data->plane[row] >> y);
            if (y >= lod_size) {
                // At top
                continue;
            }
            const u32 h = trailing_ones(data->plane[row] >> y);
            // 1 = 0b1, 2 = 0b11, 4 = 0b1111
            const u32 h_as_mask = h < 32
                ? ((u32) 1 << h) - 1
                : UINT32_MAX; // ~0
            const u32 mask = h_as_mask << y;
            // Grow horizontally
            u32 w = 1;
            while (row + w < lod_size) {
                // Fetch bits spanning height in the next row
                const u32 next_row_h = (data->plane[row + w] >> y) & h_as_mask;
                if (next_row_h != h_as_mask) {
                    // Cannot grow further horizontally
                    break;
                }
                // Unset the bits we expanded into
                data->plane[row + w] &= ~mask;
                w++;
            }
            createMesh(
                chunk,
                &data->key,
                NULL,
                NULL,
                row,
                y,
                w,
                h
            );
            y += h;
        }
    }
}
