#include "binary_greedy_mesher.h"

#include <debug.h>
#include <psxgte.h>
#include "../../../util/interface99_extensions.h"
#include "../../../util/bits.h"
#include "../../../math/math_utils.h"
#include "../../../structure/hashmap.h"
#include "../../../resources/asset_indices.h"
#include "../../../structure/primitive/primitive.h"

// Forward declarations
typedef struct World World;
IBlock* worldGetBlock(const World* world, const VECTOR* position);
IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);

#define CHUNK_SIZE_P (CHUNK_SIZE + 2)

typedef struct {
    u8 axis;
    Block* block;
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
    const int blockId = cmp(pa->key.block->id, pb->key.block->id);
    const int y = cmp(pa->key.y, pb->key.y);
    if (axis != 0) {
        return axis;
    } else if (blockId != 0) {
        return blockId;
    }
    return y;
};

u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_xxhash3(&data->key, sizeof(data->key), seed0, seed1);
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
        axis_cols[2][_y][_x] |= 1 << _z; \
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
    const u32 axial_values[2] = { 0, CHUNK_SIZE_P - 1 };
    // Z
    for (u32 z_i = 0; z_i < 2; z_i++) {
        for (u32 y = 0; y < CHUNK_SIZE_P; y++) {
            for (u32 x = 0; x < CHUNK_SIZE_P; x++) {
                const u32 z = axial_values[z_i];
                const VECTOR position = vec3_i32(
                    (chunk->position.vx * CHUNK_SIZE) + x - 1,
                    (chunk->position.vy * CHUNK_SIZE) + y - 1,
                    (chunk->position.vz * CHUNK_SIZE) + z - 1
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
    for (u32 z = 0; z < CHUNK_SIZE_P; z++) {
        for (u32 y_i = 0; y_i < 2; y_i++) {
            for (u32 x = 0; x < CHUNK_SIZE_P; x++) {
                const u32 y = axial_values[y_i];
                const VECTOR position = vec3_i32(
                    (chunk->position.vx * CHUNK_SIZE) + x - 1,
                    (chunk->position.vy * CHUNK_SIZE) + y - 1,
                    (chunk->position.vz * CHUNK_SIZE) + z - 1
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
    for (u32 z = 0; z < CHUNK_SIZE; z++) {
        for (u32 y = 0; y < CHUNK_SIZE; y++) {
            for (u32 x_i = 0; x_i < 2; x_i++) {
                const u32 x = axial_values[x_i];
                const VECTOR position = vec3_i32(
                    (chunk->position.vx * CHUNK_SIZE) + x - 1,
                    (chunk->position.vy * CHUNK_SIZE) + y - 1,
                    (chunk->position.vz * CHUNK_SIZE) + z - 1
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
    for (u32 axis = 0; axis < 3; axis++) {
        for (u32 z = 0; z < CHUNK_SIZE_P; z++) {
            for (u32 x = 0; x < CHUNK_SIZE_P; x++) {
                const u32 col = axis_cols[axis][z][x];
                // Sample descending axis, set bit to true when air meets solid
                col_face_masks[(2 * axis) + 0][z][x] = col & ~(col << 1);
                // Sample ascending axis, set bit to true when air meets solid
                col_face_masks[(2 * axis) + 1][z][x] = col & ~(col >> 1);
            }
        }
    }
    struct hashmap* data = hashmap_new(
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
    for (u32 axis = 0; axis < 6; axis++) {
        for (u32 z = 0; z < CHUNK_SIZE; z++) {
            for (u32 x = 0; x < CHUNK_SIZE; x++) {
                // Skip padding
                u32 col = col_face_masks[axis][z + 1][x + 1];
                // Remove right most padding value, always invalid
                col >>= 1;
                // Remove left most padding value, always invalid
                col &= ~(1 << CHUNK_SIZE);
                while (col != 0) {
                    u8 y = trailing_zeros(col);
                    // Clear least significant bit set
                    col &= col - 1;
                    VECTOR voxel_pos;
                    switch (axis) {
                        case 0:
                        case 1:
                            // Down, up
                            voxel_pos = vec3_i32(x, y, z);
                            break;
                        case 2:
                        case 3:
                            // Left, Right
                            voxel_pos = vec3_i32(y, z, x);
                            break;
                        default:
                            // Front, Back
                            voxel_pos = vec3_i32(x, z, y);
                            break;
                    }
                    const VECTOR block_position = vector_add(
                        voxel_pos,
                        vector_const_mul(
                            chunk->position,
                            CHUNK_SIZE
                        )
                    );
                    const IBlock* current_block = worldGetBlock(chunk->world, &block_position);
                    if (current_block == NULL) {
                        printf("[BINARY GREEDY MESHER] Null block returned constructing mask\n");
                        abort();
                        continue;
                    }
                    Block* block = VCAST_PTR(Block*, current_block);
                    PlaneMeshingData query = (PlaneMeshingData) {
                        .key = (PlaneMeshingDataKey) {
                            .axis = axis,
                            .block = block,
                            .y = y
                        },
                        .value = {0}
                    };
                    PlaneMeshingData* current = (PlaneMeshingData*) hashmap_get(data, &query);
                    if (current == NULL) {
                        hashmap_set(data, &query);
                        current = (PlaneMeshingData*) hashmap_get(data, &query);
                    }
                    current->value[x] |= 1 << z;
                }
            }
        }
    }
    chunkMeshClear(&chunk->mesh);
    size_t iter = 0;
    void* item;
    while (hashmap_iter(data, &iter, &item)) {
        const PlaneMeshingData* elem = item;
        binaryGreedyMesherConstructPlane(
            chunk,
            (FaceDirection) elem->key.axis,
            elem->key.y,
            elem->key.block,
            elem->value,
            CHUNK_SIZE
        );
    }
    hashmap_free(data);
}

u32 faceDirectionToFaceAttributeIndex(const FaceDirection face_dir) {
    u32 index = 0;
    switch (face_dir) {
        case FACE_DIR_DOWN: index = 3; break;
        case FACE_DIR_UP: index = 2; break;
        case FACE_DIR_LEFT: index = 4; break;
        case FACE_DIR_RIGHT: index = 5; break;
        case FACE_DIR_FRONT: index = 0; break;
        case FACE_DIR_BACK: index = 1; break;
    }
    return index;
}

static SMD_PRIM* createPrimitive(ChunkMesh* mesh,
                                 const Block* block,
                                 const FaceDirection face_dir,
                                 const u32 width,
                                 const u32 height) {
    cvector(SMD_PRIM) prims = mesh->p_prims;
    cvector_push_back(prims, (SMD_PRIM) {});
    mesh->p_prims = prims;
    SMD_PRIM* primitive = &prims[mesh->n_prims++];
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
    const TextureAttributes* attributes = &block->face_attributes[faceDirectionToFaceAttributeIndex(face_dir)];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = BLOCK_TEXTURE_SIZE * (face_dir == FACE_DIR_UP ? height : width);
    primitive->tv1 = BLOCK_TEXTURE_SIZE * (face_dir == FACE_DIR_UP ? width : height);
    primitive->r0 = attributes->tint.r;
    primitive->g0 = attributes->tint.g;
    primitive->b0 = attributes->tint.b;
    primitive->code = attributes->tint.cd;
    return primitive;
}

#define nextRenderAttribute(field, attribute_field, index_field, count_field) \
        cvector_push_back(mesh->attribute_field, (SVECTOR){}); \
        (primitive)->index_field = mesh->count_field; \
        (field) = &cvector_begin(mesh->attribute_field)[mesh->count_field++];

char* faceDirStr(const FaceDirection face_dir) {
    char* str = NULL;
    switch (face_dir) {
        case FACE_DIR_DOWN: str = "down";
            break;
        case FACE_DIR_UP: str = "up";
            break;
        case FACE_DIR_LEFT: str = "left";
            break;
        case FACE_DIR_RIGHT: str = "right";
            break;
        case FACE_DIR_FRONT: str = "front";
            break;
        case FACE_DIR_BACK: str = "back";
            break;
    }
    return str;
}

const INDEX _INDICES[6] = {
    {1,3,0,2},
    {3,1,2,0},
    {3,2,1,0},
    {2,3,0,1},
    {3,2,1,0},
    {2,3,0,1}
};

static void createVertices(Chunk* chunk,
                           SMD_PRIM* primitive,
                           const FaceDirection face_dir,
                           const i32 axis,
                           const i32 x,
                           const i32 y,
                           const i32 w,
                           const i32 h) {
    ChunkMesh* mesh = &chunk->mesh;
    // Construct vertices relative to chunk mesh bottom left origin
    const i16 chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    // Offset by 1 to ensure bottom block of bottom chunk starts at Y = 0
    const i16 chunk_origin_y = (-chunk->position.vy) * CHUNK_SIZE;
    const i16 chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
    const SVECTOR chunk_origin = vec3_i16(
        chunk_origin_x,
        chunk_origin_y,
        chunk_origin_z
    );
#define createVertex(_x, _y) ({ \
    const SVECTOR face_dir_pos = faceDirectionPosition(face_dir, axis, (_x), (_y)); \
    (SVECTOR) { \
        (chunk_origin_x + face_dir_pos.vx) * BLOCK_SIZE, \
        (chunk_origin_y - face_dir_pos.vy) * BLOCK_SIZE, \
        (chunk_origin_z + face_dir_pos.vz) * BLOCK_SIZE, \
    }; \
})
    const SVECTOR vertices[4] = {
        [0] = createVertex(x, y),
        [1] = createVertex(x + w, y),
        [2] = createVertex(x, y + h),
        [3] = createVertex(x + w, y + h)
    };
    // DEBUG_LOG(
    //     "[MESH] Face: %s, Axis: %d Base: (%d,%d), Dims: (%d,%d)\n",
    //     faceDirStr(face_dir),
    //     axis,
    //     x, y,
    //     w, h
    // );
    const INDEX indices = _INDICES[face_dir];
    SVECTOR* vertex;
    SVECTOR* current_vertex;
    #define bindVertex(v) nextRenderAttribute(vertex, p_verts, v, n_verts); \
        current_vertex = &vertices[indices.v]; \
        vertex->vx = current_vertex->vx; \
        vertex->vy = current_vertex->vy; \
        vertex->vz = current_vertex->vz
    bindVertex(v0);
    bindVertex(v1);
    bindVertex(v2);
    bindVertex(v3);}

static void createNormal(ChunkMesh* mesh,
                         SMD_PRIM* primitive,
                         const FaceDirection face_dir) {
    SVECTOR* norm = NULL;
    nextRenderAttribute(norm, p_norms, n0, n_norms);
#define normal(x,y,z) norm->vx = x * ONE; norm->vy = y * ONE; norm->vz = z * ONE
    switch (face_dir) {
        case FACE_DIR_DOWN: normal(0, 1, 0); break;
        case FACE_DIR_UP: normal(0, -1, 0); break;
        case FACE_DIR_LEFT: normal(-1, 0, 0); break;
        case FACE_DIR_RIGHT: normal(1, 0, 0); break;
        case FACE_DIR_FRONT: normal(0, 0, -1); break;
        case FACE_DIR_BACK: normal(0, 0, 1); break;
    };
}

static void createQuad(Chunk* chunk,
                              const FaceDirection face_dir,
                              const u32 axis,
                              const Block* block,
                              const u32 x,
                              const u32 y,
                              const u32 w,
                              const u32 h) {
    SMD_PRIM* primitive = createPrimitive(
        &chunk->mesh,
        block,
        face_dir,
        w,
        h
    );
    createVertices(
        chunk,
        primitive,
        face_dir,
        (i32) axis,
        x,
        y,
        w,
        h
    );
    createNormal(
        &chunk->mesh,
        primitive,
        face_dir
    );
}

void binaryGreedyMesherConstructPlane(Chunk* chunk,
                                      const FaceDirection face_dir,
                                      const u32 axis,
                                      const Block* block,
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
                chunk,
                face_dir,
                axis,
                block,
                row,
                y,
                w,
                h
            );
            y += h;
        }
    }
}

SVECTOR faceDirectionPosition(const FaceDirection face_dir, const i32 axis, const i32 x, const i32 y) {
    SVECTOR position = {0};
    switch (face_dir) {
        case FACE_DIR_DOWN: position = vec3_i16(x, axis + 1, y); break;
        case FACE_DIR_UP: position = vec3_i16(x, axis, y); break;
        case FACE_DIR_LEFT: position = vec3_i16(axis, y, x); break;
        case FACE_DIR_RIGHT: position = vec3_i16(axis + 1, y, x); break;
        case FACE_DIR_FRONT: position = vec3_i16(x, y, axis); break;
        case FACE_DIR_BACK: position = vec3_i16(x, y, axis + 1); break;
    }
    return position;
}
