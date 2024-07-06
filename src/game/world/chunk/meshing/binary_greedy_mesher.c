#include "binary_greedy_mesher.h"

#include <psxgte.h>

#include "../../../logging/logging.h"
#include "../../../util/interface99_extensions.h"
#include "../../../util/bits.h"
#include "../../../math/vector.h"
#include "../../../structure/hashmap.h"
#include "../../../resources/asset_indices.h"
#include "../../../structure/primitive/primitive.h"
#include "../../../resources/assets.h"
#include "../../../structure/primitive/cube.h"
#include "../../position.h"

// Forward declarations
FWD_DECL typedef struct World World;
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);
FWD_DECL IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);
FWD_DECL IBlock* chunkGetBlock(const Chunk* chunk, i32 x, i32 y, i32 z);

#define CHUNK_SIZE_PADDED (CHUNK_SIZE + 2)
#define AXIS_COUNT 3
#define AXIAL_EDGES_COUNT 2
static const u32 AXIAL_EDGES[AXIAL_EDGES_COUNT] = { 0, CHUNK_SIZE_PADDED - 1 };

typedef u32 FacesColumns[FACE_DIRECTION_COUNT][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED];

INLINE void addVoxelToFaceColumns(FacesColumns axis_cols,
                                  FacesColumns axis_cols_opaque,
                                  const IBlock* iblock,
                                  const u32 x,
                                  const u32 y,
                                  const u32 z) {
    if (iblock == NULL) {
        return;
    }
    const Block* block = VCAST_PTR(Block*, iblock);
    if (blockGetType(block->id) == BLOCKTYPE_EMPTY) {
        return;
    }
    axis_cols[FACE_DIR_DOWN][z][x] |= 1 << y;
    axis_cols[FACE_DIR_UP][z][x] |= 1 << y;
    axis_cols[FACE_DIR_LEFT][y][z] |= 1 << x;
    axis_cols[FACE_DIR_RIGHT][y][z] |= 1 << x;
    axis_cols[FACE_DIR_BACK][y][x] |= 1 << z;
    axis_cols[FACE_DIR_FRONT][y][x] |= 1 << z;
    const u8 bitset = VCALL(*iblock, opaqueBitset);
#define bitsetAt(i) ((bitset >> (i)) & 0b1)
    axis_cols_opaque[FACE_DIR_DOWN][z][x] |= bitsetAt(1) << y;
    axis_cols_opaque[FACE_DIR_UP][z][x] |= bitsetAt(0) << y;
    axis_cols_opaque[FACE_DIR_LEFT][y][z] |= bitsetAt(3) << x;
    axis_cols_opaque[FACE_DIR_RIGHT][y][z] |= bitsetAt(2) << x;
    axis_cols_opaque[FACE_DIR_BACK][y][x] |= bitsetAt(4) << z;
    axis_cols_opaque[FACE_DIR_FRONT][y][x] |= bitsetAt(5) << z;
#undef bitsetAt
}

void binaryGreedyMesherBuildMesh(Chunk* chunk, const BreakingState* breaking_state) {
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
                    const VECTOR world_block_position = vector_add(
                        vector_const_mul(
                            chunk_block_position.chunk,
                            CHUNK_SIZE
                        ),
                        chunk_block_position.block
                    );
                    if (breaking_state != NULL
                        && breaking_state->block != NULL
                        && vec3_equal(breaking_state->position, world_block_position)) {
                        // Don't generate faces that match with the breaking block
                        // as we will create those later with the overlay. This avoids
                        // z fighting between mesh faces and overlay faces.
                        continue;
                    }
                    const IBlock* current_block = worldGetChunkBlock(chunk->world, &chunk_block_position);
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
                    const PlaneMeshingData query = (PlaneMeshingData) {
                        .key = (PlaneMeshingDataKey) { face, y, block },
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
            elem->key.face,
            elem->key.axis,
            elem->key.block,
            elem->plane,
            CHUNK_SIZE
        );
    }
    hashmap_free(data);
    if (breaking_state != NULL && breaking_state->block != NULL) {
        binaryGreedyMesherConstructBreakingOverlay(chunk, breaking_state);
    }
}

static MeshPrimitive* createPrimitive(ChunkMesh* mesh,
                                      const Block* block,
                                      const FaceDirection face_dir,
                                      const u32 width,
                                      const u32 height,
                                      const Texture* texture_override,
                                      const TextureAttributes texture_attributes_override[FACE_DIRECTION_COUNT]) {
    Mesh* face_dir_mesh = &mesh->face_meshes[face_dir];
    cvector(MeshPrimitive) prims = face_dir_mesh->p_prims;
    cvector_push_back(prims, (MeshPrimitive){});
    face_dir_mesh->p_prims = prims;
    MeshPrimitive* primitive = &prims[face_dir_mesh->n_prims++];
    primitive->type = MESH_PRIM_TYPE_QUAD;
    const Texture* texture = texture_override != NULL
        ? texture_override
        : &textures[ASSET_TEXTURES_TERRAIN_INDEX];
    primitive->tpage = texture->tpage;
    primitive->clut = texture->clut;
    const TextureAttributes* attributes = texture_attributes_override != NULL
        ? &texture_attributes_override[face_dir]
        : &block->face_attributes[face_dir];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = BLOCK_TEXTURE_SIZE * width;
    primitive->tv1 = BLOCK_TEXTURE_SIZE * height;
    primitive->r = attributes->tint.r;
    primitive->g = attributes->tint.g;
    primitive->b = attributes->tint.b;
    primitive->tint= attributes->tint.cd;
    return primitive;
}

#define nextRenderAttribute(mesh, attribute_field, index_field, count_field) ({ \
        cvector_push_back((mesh)->attribute_field, (SVECTOR){}); \
        primitive->index_field = (mesh)->count_field; \
        (&cvector_begin((mesh)->attribute_field)[(mesh)->count_field++]); \
})

static const INDEX INDICES[FACE_DIRECTION_COUNT] = {
    {3,2,1,0},
    {1,0,3,2},
    {3,2,1,0},
    {2,3,0,1},
    {2,3,0,1},
    {3,2,1,0}
};

static SVECTOR createVertex(const i32 chunk_origin_x,
                            const i32 chunk_origin_y,
                            const i32 chunk_origin_z,
                            const FaceDirection face_dir,
                            const i32 axis,
                            const i32 x,
                            const i32 y) {
    SVECTOR vertex = {0};
#define pos(_x, _y, _z) vec3_i16( \
    (chunk_origin_x + (_x)) * BLOCK_SIZE, \
    (chunk_origin_y - (_y)) * BLOCK_SIZE, \
    (chunk_origin_z + (_z)) * BLOCK_SIZE \
)
    switch (face_dir) {
        case FACE_DIR_DOWN: vertex = pos(x, axis, y); break;
        case FACE_DIR_UP: vertex = pos(x, axis + 1, y); break;
        case FACE_DIR_LEFT: vertex = pos(axis, y, x); break;
        case FACE_DIR_RIGHT: vertex = pos(axis + 1, y, x); break;
        case FACE_DIR_BACK: vertex = pos(x, y, axis); break;
        case FACE_DIR_FRONT: vertex = pos(x, y, axis + 1); break;
    }
#undef pos
    return vertex;
}

static void createVertices(Chunk* chunk,
                           MeshPrimitive* primitive,
                           const FaceDirection face_dir,
                           const i32 axis,
                           const i32 x,
                           const i32 y,
                           const i32 w,
                           const i32 h) {
    Mesh* const mesh = &chunk->mesh.face_meshes[face_dir];
    // Construct vertices relative to chunk mesh bottom left origin
    const i32 chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    const i32 chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
    const i32 chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
#define _createVertex(_x, _y) createVertex( \
    chunk_origin_x, \
    chunk_origin_y, \
    chunk_origin_z, \
    face_dir, \
    axis, \
    (_x), \
    (_y) \
)
    const SVECTOR vertices[4] = {
        [0] = _createVertex(x, y),
        [1] = _createVertex(x + w, y),
        [2] = _createVertex(x, y + h),
        [3] = _createVertex(x + w, y + h)
    };
#undef _createVertex
    const INDEX indices = INDICES[face_dir];
#define bindVertex(v) *nextRenderAttribute(mesh, p_verts, v, n_verts) = vertices[indices.v]
    bindVertex(v0);
    bindVertex(v1);
    bindVertex(v2);
    bindVertex(v3);
#undef bindVertex
}

static void createNormal(ChunkMesh* mesh,
                         MeshPrimitive* primitive,
                         const FaceDirection face_dir) {
    SVECTOR* norm = nextRenderAttribute(&mesh->face_meshes[face_dir], p_norms, n0, n_norms);
#undef nextRenderAttribute
    *norm = svector_const_mul(
        FACE_DIRECTION_NORMALS[face_dir],
        ONE
    );
}

static void createQuad(Chunk* chunk,
                       const FaceDirection face_dir,
                       const u32 axis,
                       const Block* block,
                       const Texture* texture_override,
                       const TextureAttributes* texture_attributes_override,
                       const u32 x,
                       const u32 y,
                       const u32 w,
                       const u32 h) {
    MeshPrimitive* primitive = createPrimitive(
        &chunk->mesh,
        block,
        face_dir,
        w,
        h,
        texture_override,
        texture_attributes_override
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

void binaryGreedyMesherConstructBreakingOverlay(Chunk* chunk, const BreakingState* breaking_state) {
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(
        &breaking_state->position,
        CHUNK_SIZE
    );
    const u8 visible_sides_bitset = breaking_state->visible_sides_bitset;
    const Block* block = VCAST_PTR(Block*, breaking_state->block);
    TextureAttributes texture_attributes[FACE_DIRECTION_COUNT] = {0};
    #pragma GCC unroll 6
    for (FaceDirection face_dir = 0; face_dir < FACE_DIRECTION_COUNT; face_dir++) {
        if (((visible_sides_bitset >> face_dir) & 0b1) == 0) {
            continue;
        }
        // Creating these as we go is fine since we will only ever
        // reference the one we create now in this loop iteration.
        // The previous and next attributes that have/will be created
        // will never be used in the current loop.
        FaceDirection current_face_dir;
        switch (face_dir) {
            case FACE_DIR_FRONT: current_face_dir = FACE_DIR_BACK; break;
            case FACE_DIR_BACK: current_face_dir = FACE_DIR_FRONT; break;
            default: current_face_dir = face_dir; break;
        }
        TextureAttributes* attributes = &texture_attributes[current_face_dir];
        attributes->u = face_dir * BLOCK_TEXTURE_SIZE;
        attributes->v = 0;
        attributes->w = BLOCK_TEXTURE_SIZE;
        attributes->h = BLOCK_TEXTURE_SIZE;
        attributes->tint = block->face_attributes[face_dir].tint;
        const Texture texture = (Texture) {
            .tpage = getTPage(
                2,
                1,
                breaking_texture_offscreen.x,
                breaking_texture_offscreen.y
            ),
            .clut = textures[ASSET_TEXTURES_TERRAIN_INDEX].clut
        };
        u32 x = 0;
        u32 y = 0;
        u32 axis = 0;
        #define pos(_axis, _x, _y) \
            axis = chunk_block_position.block.v##_axis; \
            x = chunk_block_position.block.v##_x; \
            y = chunk_block_position.block.v##_y
        switch (current_face_dir) {
            case FACE_DIR_DOWN:
            case FACE_DIR_UP:
                pos(y, x, z);
                break;
            case FACE_DIR_LEFT:
            case FACE_DIR_RIGHT:
                pos(x, z, y);
                break;
            case FACE_DIR_BACK:
            case FACE_DIR_FRONT:
                pos(z, x, y);
                break;
        }
        createQuad(
            chunk,
            current_face_dir,
            // This should be the coordinate value for which ever axis stays constant
            // in the facing direction, i.e. in the UP direction the Y coord doesn't
            // change at each vertex, that would be the value here.
            axis,
            block,
            &texture,
            texture_attributes,
            // These should be in winding order, see <src/structure/primitive/cube_layout.md>
            x,
            y,
            1,
            1
        );
    }
}
