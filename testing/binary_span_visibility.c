#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "hashmap.h"

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

#define INT8_BIN_PATTERN "%c%c%c%c%c%c%c%c"
#define INT8_BIN_LAYOUT(i) \
    ((i) & 0x80 ? '1' : '0'), \
    ((i) & 0x40 ? '1' : '0'), \
    ((i) & 0x20 ? '1' : '0'), \
    ((i) & 0x10 ? '1' : '0'), \
    ((i) & 0x08 ? '1' : '0'), \
    ((i) & 0x04 ? '1' : '0'), \
    ((i) & 0x02 ? '1' : '0'), \
    ((i) & 0x01 ? '1' : '0')

#define INT16_BIN_PATTERN INT8_BIN_PATTERN INT8_BIN_PATTERN
#define INT32_BIN_PATTERN INT16_BIN_PATTERN INT16_BIN_PATTERN
#define INT16_BIN_LAYOUT(i) INT8_BIN_LAYOUT((i) >> 8), INT8_BIN_LAYOUT(i)
#define INT32_BIN_LAYOUT(i) INT16_BIN_LAYOUT((i) >> 16), INT16_BIN_LAYOUT(i)

#define VEC_PATTERN "(%d,%d,%d)"
#define VEC_LAYOUT(v) (v).vx, (v).vy, (v).vz
#define VEC_PTR_LAYOUT(v) (v)->vx, (v)->vy, (v)->vz

#define MAT_PATTERN "%d, %d, %d | %d,\n%d, %d, %d | %d,\n%d, %d, %d | %d\n"
#define MAT_LAYOUT(_m) (_m).m[0][0], (_m).m[0][1], (_m).m[0][2], (_m).t[0], \
    (_m).m[1][0], (_m).m[1][1], (_m).m[1][2], (_m).t[1], \
    (_m).m[2][0], (_m).m[2][1], (_m).m[2][2], (_m).t[2]
#define MAT_PTR_LAYOUT(_m) (_m)->m[0][0], (_m)->m[0][1], (_m)->m[0][2], (_m)->t[0], \
    (_m)->m[1][0], (_m)->m[1][1], (_m)->m[1][2], (_m)->t[1], \
    (_m)->m[2][0], (_m)->m[2][1], (_m)->m[2][2], (_m)->t[2]

#define INT64_PATTERN "0x%08x%08x"
#define INT64_LAYOUT(v) (u32) (((u64)(v)) >> 32), (u32) (v)

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

#define _vec2_layout(x, y) .vx = (x), .vy = (y)
#define _vec3_layout(x, y, z) _vec2_layout(x, y), .vz = (z)
#define vec3_i64(x, y, z) ((LVECTOR) { _vec3_layout(x, y, z) })
#define vec3_i32(x, y, z) ((VECTOR) { _vec3_layout(x, y, z) })
#define vec3_i16(x, y, z) ((SVECTOR) { _vec3_layout(x, y, z) })
#define vec3_i8(x, y, z) ((IBVECTOR) { _vec3_layout(x, y, z) })
#define vec3_u8(x, y, z) ((BVECTOR) { _vec3_layout(x, y, z) })
#define vec2_i16(x, y) ((DVECTOR) { _vec2_layout(x, y) })
#define vec3_rgb(_r, _g, _b) ((CVECTOR) { .r = (_r), .g = (_g), .b = (_b) })

#define vec3_i32_all(v) vec3_i32(v, v, v)
#define vec3_i16_all(v) vec3_i16(v, v, v)
#define vec3_i8_all(v) vec3_i8(v, v, v)
#define vec3_rgb_all(v) vec3_rgb(v, v, v)
#define vec2_i16_all(v) vec2_i16(v, v)

#define _isPowerOf2(x) (((_x) & ((_x) - 1)) == 0)
#define isPowerOf2(x) ({ \
    __typeof__(x) _x = (x); \
    _isPowerOf2(_x); \
})

#define cmp(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_b > _a) - (_b < _a); \
})

#define GLUE(x,y) x##y

u8 trailing_zeros(u32 value) {
    // Ripped from: https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
    // If value is 1101000 (base 2), then c will be 3
    // NOTE: if value == 0, then c = 31.
    if (value & 0x1) {
        // special case for odd v (assumed to happen half of the time)
        return 0;
    }
    u32 c = 1; // Number of zero bits on the right
    if ((value & 0xffff) == 0) {
        value >>= 16;
        c += 16;
    }
    if ((value & 0xff) == 0) {
        value >>= 8;
        c += 8;
    }
    if ((value & 0xf) == 0) {
        value >>= 4;
        c += 4;
    }
    if ((value & 0x3) == 0) {
        value >>= 2;
        c += 2;
    }
    return c - (value & 0x1);
}

u8 trailing_ones(u32 value) {
    // Ripped from: https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
    // If value is 0010111(base 2), then c will be 3
    // NOTE: if value == 0xffffffff, then c = 31.
    if (value == 0) {
        // special case for odd v (assumed to happen half of the time)
        return 0;
    }
    u32 c = 1; // Number of zero bits on the right
    if ((value & 0xffff) == 0xffff) {
        value >>= 16;
        c += 16;
    }
    if ((value & 0xff) == 0xff) {
        value >>= 8;
        c += 8;
    }
    if ((value & 0xf) == 0xf) {
        value >>= 4;
        c += 4;
    }
    if ((value & 0x3) == 0x3) {
        value >>= 2;
        c += 2;
    }
    return c - !(value & 0x1);
}

#define CHUNK_SIZE 8

typedef enum EBlockID {
    BLOCK_ID_AIR = 0,
    BLOCK_ID_STONE,
} EBlockID;

typedef struct Block {
    u8 id;
    bool transparent;
} Block;

#define blockEquals(b0, b1) ((b0)->id == (b1)->id)

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
#if defined(CHUNK_SIZE) && CHUNK_SIZE > 0 && CHUNK_SIZE <= 32 && _isPowerOf2(CHUNK_SIZE)
    #define planeType(size, name) typedef GLUE(u, size) name[size]
    planeType(CHUNK_SIZE,BinaryMeshPlane);
#undef planeType
#else
#error CHUNK_SIZE must be in the interval (0, 32] and be a power of 2
#endif

typedef struct {
    const u8 face;
    const u8 axis;
    const Block* block;
} PlaneMeshingDataKey;

typedef struct {
    PlaneMeshingDataKey key;
    BinaryMeshPlane plane;
} PlaneMeshingData;

int plane_meshing_data_compare(const void* a, const void* b, void* ignored) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    const int axis = cmp(pa->key.face, pb->key.face);
    // TODO: We need to account for direction with blocks that have
    //       non-block models or non-uniform face textures
    const bool block_id = blockEquals(pa->key.block, pb->key.block);
    const int y = cmp(pa->key.axis, pb->key.axis);
    if (axis != 0) {
        return axis;
    } else if (block_id) {
        return 0;
    }
    return y;
};

u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_xxhash3(&data->key, sizeof(PlaneMeshingDataKey), seed0, seed1);
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
    if (block->id == BLOCK_ID_AIR) {
        return;
    }
    axis_cols[FACE_DIR_DOWN][z][x] |= 1 << y;
    axis_cols[FACE_DIR_UP][z][x] |= 1 << y;
    axis_cols[FACE_DIR_LEFT][y][z] |= 1 << x;
    axis_cols[FACE_DIR_RIGHT][y][z] |= 1 << x;
    axis_cols[FACE_DIR_BACK][y][x] |= 1 << z;
    axis_cols[FACE_DIR_FRONT][y][x] |= 1 << z;
    const u8 bitset = block->transparent ? 0 : 0b1111111;
#define bitsetAt(i) ((bitset >> (i)) & 0b1)
    axis_cols_opaque[FACE_DIR_DOWN][z][x] |= bitsetAt(1) << y;
    axis_cols_opaque[FACE_DIR_UP][z][x] |= bitsetAt(0) << y;
    axis_cols_opaque[FACE_DIR_LEFT][y][z] |= bitsetAt(3) << x;
    axis_cols_opaque[FACE_DIR_RIGHT][y][z] |= bitsetAt(2) << x;
    axis_cols_opaque[FACE_DIR_BACK][y][x] |= bitsetAt(4) << z;
    axis_cols_opaque[FACE_DIR_FRONT][y][x] |= bitsetAt(5) << z;
#undef bitsetAt
}

static void createMesh(Chunk* chunk,
                       const PlaneMeshingDataKey* data,
                       const u32 x,
                       const u32 y,
                       const u32 w,
                       const u32 h);

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
                row,
                y,
                w,
                h
            );
            y += h;
        }
    }
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
                    VECTOR world_block_position = vec3_i32_all(0);
                    switch (face) {
                        case FACE_DIR_DOWN:
                            world_block_position = vec3_i32(x, y, z);
                            break;
                        case FACE_DIR_UP:
                            world_block_position = vec3_i32(x, y, z);
                            break;
                        case FACE_DIR_LEFT:
                            world_block_position = vec3_i32(y, z, x);
                            break;
                        case FACE_DIR_RIGHT:
                            world_block_position = vec3_i32(y, z, x);
                            break;
                        case FACE_DIR_BACK:
                            world_block_position = vec3_i32(x, z, y);
                            break;
                        case FACE_DIR_FRONT:
                            world_block_position = vec3_i32(x, z, y);
                            break;
                    }
                    Block* block = worldGetBlock(
                        chunk,
                        &world_block_position
                    );
                    if (block == NULL) {
                        printf(
                            "[BINARY GREEDY MESHER] Null block returned while constructing mask [Face: %d] [Chunk: " VEC_PATTERN "] [Block: " VEC_PATTERN "]\n",
                            face,
                            0, 0, 0,
                            VEC_LAYOUT(world_block_position)
                        );
                        continue;
                    }
                    if (block->id == BLOCK_ID_AIR) {
                        printf(
                            "[BINARY GREEDY MESHER] Empty block encountered while constructing mask [Face: %d] [Chunk: " VEC_PATTERN "] [Block: " VEC_PATTERN "]\n",
                            face,
                            0,0,0,
                            VEC_LAYOUT(world_block_position)
                        );
                        continue;
                    }
                    const PlaneMeshingData query = (PlaneMeshingData) {
                        .key = (PlaneMeshingDataKey) {
                            face,
                            y,
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

static void createMesh(Chunk* chunk,
                       const PlaneMeshingDataKey* data,
                       const u32 x,
                       const u32 y,
                       const u32 w,
                       const u32 h) {

}
