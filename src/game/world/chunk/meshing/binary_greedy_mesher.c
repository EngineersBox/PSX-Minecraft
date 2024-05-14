#include "binary_greedy_mesher.h"

#include <psxgte.h>
#include "../../../../logging/logging.h"
#include "../../../util/interface99_extensions.h"
#include "../../../util/bits.h"
#include "../../../math/math_utils.h"
#include "../../../structure/hashmap.h"
#include "../../../resources/asset_indices.h"
#include "../../../structure/primitive/primitive.h"

// Forward declarations
FWD_DECL typedef struct World World;
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);
FWD_DECL IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);
FWD_DECL IBlock* chunkGetBlock(const Chunk* chunk, i32 x, i32 y, i32 z);

#define CHUNK_SIZE_PADDED (CHUNK_SIZE + 2)
#define AXIS_COUNT 3
#define FACES_COUNT (AXIS_COUNT * 2)
#define AXIAL_EDGES_COUNT 2
static const u32 AXIAL_EDGES[AXIAL_EDGES_COUNT] = { 0, CHUNK_SIZE_PADDED - 1 };

typedef u32 FacesColumns[FACES_COUNT][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED];

__attribute__((always_inline, gnu_inline))
inline void addVoxelToFaceColumns(FacesColumns axis_cols,
                               FacesColumns axis_cols_opaque,
                               const IBlock* iblock,
                               const u32 x,
                               const u32 y,
                               const u32 z) {
    if (iblock == NULL) {
        return;
    }
    const Block* block = VCAST_PTR(Block*, iblock);
    if (block->type == BLOCKTYPE_EMPTY) {
        return;
    }
    axis_cols[0][z][x] |= 1 << y; // DOWN
    axis_cols[1][z][x] |= 1 << y; // UP
    axis_cols[2][y][z] |= 1 << x; // LEFT
    axis_cols[3][y][z] |= 1 << x; // RIGHT
    axis_cols[4][y][x] |= 1 << z; // BACK
    axis_cols[5][y][x] |= 1 << z; // FRONT
    const u8 bitset = VCALL(*iblock, opaqueBitset);
#define bitsetAt(i) ((bitset >> (i)) & 0x1)
    axis_cols_opaque[0][z][x] |= bitsetAt(1) << y;
    axis_cols_opaque[1][z][x] |= bitsetAt(0) << y;
    axis_cols_opaque[2][y][z] |= bitsetAt(3) << x;
    axis_cols_opaque[3][y][z] |= bitsetAt(2) << x;
    axis_cols_opaque[4][y][x] |= bitsetAt(4) << z;
    axis_cols_opaque[5][y][x] |= bitsetAt(5) << z;
#undef bitset
}

void binaryGreedyMesherBuildMesh(Chunk* chunk) {
    FacesColumns faces_cols = {0};
    // See binary_greedy_mesher_transparency.md
    FacesColumns faces_cols_opaque = {0};
    FacesColumns col_face_masks = {0};
    // Inner chunk blocks
    for (u32 z = 0; z < CHUNK_SIZE; z++) {
        for (u32 x = 0; x < CHUNK_SIZE; x++) {
            for (u32 y = 0; y < CHUNK_SIZE; y++) {
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    chunkGetBlock(chunk, x, y, z),
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
                const VECTOR position = vec3_i32(
                    (chunk->position.vx * CHUNK_SIZE) + x - 1,
                    (chunk->position.vy * CHUNK_SIZE) + y - 1,
                    (chunk->position.vz * CHUNK_SIZE) + z - 1
                );
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    worldGetBlock(chunk->world, &position),
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
                const VECTOR position = vec3_i32(
                    (chunk->position.vx * CHUNK_SIZE) + x - 1,
                    (chunk->position.vy * CHUNK_SIZE) + y - 1,
                    (chunk->position.vz * CHUNK_SIZE) + z - 1
                );
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    worldGetBlock(chunk->world, &position),
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
                const VECTOR position = vec3_i32(
                    (chunk->position.vx * CHUNK_SIZE) + x - 1,
                    (chunk->position.vy * CHUNK_SIZE) + y - 1,
                    (chunk->position.vz * CHUNK_SIZE) + z - 1
                );
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    worldGetBlock(chunk->world, &position),
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
    for (u32 face = 0; face < FACES_COUNT; face++) {
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
                    switch (face) {
                        case FACE_DIR_DOWN:
                        case FACE_DIR_UP:
                            chunk_block_position.block = vec3_i32(x, y, z);
                            break;
                        case FACE_DIR_LEFT:
                        case FACE_DIR_RIGHT:
                            chunk_block_position.block = vec3_i32(y, z, x);
                            break;
                        case FACE_DIR_BACK:
                        case FACE_DIR_FRONT:
                        default:
                            chunk_block_position.block = vec3_i32(x, z, y);
                            break;
                    }
                    const IBlock* current_block = worldGetChunkBlock(chunk->world, &chunk_block_position);
                    if (current_block == NULL) {
                        errorAbort(
                            "[BINARY GREEDY MESHER] Null block returned while constructing mask [Face: %d] [Chunk: (%d,%d,%d)] [Block: (%d,%d,%d)]\n",
                            face,
                            inlineVec(chunk->position),
                            inlineVec(chunk_block_position.block)
                        );
                        continue;
                    }
                    const Block* block = VCAST_PTR(Block*, current_block);
                    if (block->type == BLOCKTYPE_EMPTY) {
                        errorAbort(
                            "[BINARY GREEDY MESHER] Empty block encountered while constructing mask [Face: %d] [Chunk: (%d,%d,%d)] [Block: (%d,%d,%d)]\n",
                            face,
                            inlineVec(chunk->position),
                            inlineVec(chunk_block_position.block)
                        );
                        continue;
                    }
                    const PlaneMeshingData query = (PlaneMeshingData) {
                        .key = (PlaneMeshingDataKey) { face, y, block },
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
        PlaneMeshingData* elem = item;
        binaryGreedyMesherConstructPlane(
            chunk,
            elem->key.face,
            elem->key.axis,
            elem->key.block,
            elem->value,
            CHUNK_SIZE
        );
    }
    hashmap_free(data);
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
    const TextureAttributes* attributes = &block->face_attributes[face_dir];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = BLOCK_TEXTURE_SIZE * width;
    primitive->tv1 = BLOCK_TEXTURE_SIZE * height;
    primitive->r0 = attributes->tint.r;
    primitive->g0 = attributes->tint.g;
    primitive->b0 = attributes->tint.b;
    primitive->code = attributes->tint.cd;
    return primitive;
}

#define nextRenderAttribute(attribute_field, index_field, count_field) ({ \
        cvector_push_back(mesh->attribute_field, (SVECTOR){}); \
        (primitive)->index_field = mesh->count_field; \
        (&cvector_begin(mesh->attribute_field)[mesh->count_field++]); \
})

static const INDEX INDICES[FACES_COUNT] = {
    {3,2,1,0},
    {1,0,3,2},
    {3,2,1,0},
    {2,3,0,1},
    {2,3,0,1},
    {3,2,1,0}
};

static void createVertices(Chunk* chunk,
                           SMD_PRIM* primitive,
                           const FaceDirection face_dir,
                           const i32 axis,
                           const i32 x,
                           const i32 y,
                           const i32 w,
                           const i32 h) {
    ChunkMesh* const mesh = &chunk->mesh;
    // Construct vertices relative to chunk mesh bottom left origin
    const i32 chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    const i32 chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
    const i32 chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
#define createVertex(_x, _y) ({ \
    const SVECTOR face_dir_pos = faceDirectionPosition(face_dir, axis, (_x), (_y)); \
    vec3_i16( \
        (chunk_origin_x + face_dir_pos.vx) * BLOCK_SIZE, \
        (chunk_origin_y - face_dir_pos.vy) * BLOCK_SIZE, \
        (chunk_origin_z + face_dir_pos.vz) * BLOCK_SIZE \
    ); \
})
    const SVECTOR vertices[4] = {
        [0] = createVertex(x, y),
        [1] = createVertex(x + w, y),
        [2] = createVertex(x, y + h),
        [3] = createVertex(x + w, y + h)
    };
#undef createVertex
    const INDEX indices = INDICES[face_dir];
#define bindVertex(v) *nextRenderAttribute(p_verts, v, n_verts) = vertices[indices.v]
    bindVertex(v0);
    bindVertex(v1);
    bindVertex(v2);
    bindVertex(v3);
#undef bindVertex
}

static void createNormal(ChunkMesh* mesh,
                         SMD_PRIM* primitive,
                         const FaceDirection face_dir) {
    SVECTOR* norm = nextRenderAttribute(p_norms, n0, n_norms);
#undef nextRenderAttribute
#define normal(x,y,z) norm->vx = x * ONE; norm->vy = y * ONE; norm->vz = z * ONE
    switch (face_dir) {
        case FACE_DIR_DOWN: normal(0, 1, 0); break;
        case FACE_DIR_UP: normal(0, -1, 0); break;
        case FACE_DIR_LEFT: normal(-1, 0, 0); break;
        case FACE_DIR_RIGHT: normal(1, 0, 0); break;
        case FACE_DIR_FRONT: normal(0, 0, 1); break;
        case FACE_DIR_BACK: normal(0, 0, -1); break;
    }
#undef normal
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
        axis,
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
            const u32 h_as_mask = h < 32
                ? (1 << h) - 1
                : UINT32_MAX; // ~0
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
        case FACE_DIR_DOWN: position = vec3_i16(x, axis, y); break;
        case FACE_DIR_UP: position = vec3_i16(x, axis + 1, y); break;
        case FACE_DIR_LEFT: position = vec3_i16(axis, y, x); break;
        case FACE_DIR_RIGHT: position = vec3_i16(axis + 1, y, x); break;
        case FACE_DIR_BACK: position = vec3_i16(x, y, axis); break;
        case FACE_DIR_FRONT: position = vec3_i16(x, y, axis + 1); break;
    }
    return position;
}

