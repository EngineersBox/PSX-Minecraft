#include "binary_greedy_mesher.h"

#include <psxgte.h>

#include "../../../../logging/logging.h"
#include "../../../../lighting/lightmap.h"
#include "../../../../util/interface99_extensions.h"
#include "../../../../util/bits.h"
#include "../../../../util/preprocessor.h"
#include "../../../../math/vector.h"
#include "../../../../structure/hashmap.h"
#include "../../../../resources/asset_indices.h"
#include "../../../../structure/primitive/primitive.h"
#include "../../../../resources/assets.h"
#include "../../position.h"
#include "../chunk_structure.h"
#include "plane_meshing_data.h"

// Forward declarations
FWD_DECL typedef struct World World;
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);
FWD_DECL IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position);
FWD_DECL IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position);
FWD_DECL IBlock* chunkGetBlock(const Chunk* chunk, i32 x, i32 y, i32 z);
FWD_DECL void chunkSetLightValue(Chunk* chunk,
                                 const VECTOR* position,
                                 const LightLevel light_value,
                                 const LightType light_type);
FWD_DECL LightLevel worldGetLightValue(const World* world, const VECTOR* position);

#define CHUNK_SIZE_PADDED (CHUNK_SIZE + 2)
#define AXIS_COUNT 3
#define AXIAL_EDGES_COUNT 2
static const u32 AXIAL_EDGES[AXIAL_EDGES_COUNT] = { 0, CHUNK_SIZE_PADDED - 1 };

typedef u32 FacesColumns[FACE_DIRECTION_COUNT][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED];

#if defined(CHUNK_SIZE) && CHUNK_SIZE > 0 && CHUNK_SIZE <= 32 && _isPowerOf2(CHUNK_SIZE)
    #define bitmapType(size, name) \
        typedef GLUE(u, size) ChunkBitmapBitset; \
        typedef ChunkBitmapBitset name[size * size]
    bitmapType(CHUNK_SIZE, ChunkBitmap);
    #undef bitmapType
#else
#error CHUNK_SIZE must be in the interval (0, 32] and be a power of 2
#endif

#define chunkBitmapGetBit(bitmap, pos) (((bitmap)[((pos)->vy * CHUNK_SIZE) + (pos)->vz] >> (pos)->vx) & 0b1)
#define chunkBitmapSetBit(bitmap, pos) ((bitmap)[((pos)->vy * CHUNK_SIZE) + (pos)->vz] |= 1 << (pos)->vx)

INLINE static void addVoxelToFaceColumns(FacesColumns axis_cols,
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
    const u8 bitset = blockGetOpacityBitset(
        block->id,
        block->orientation
    );
#define bitsetAt(i) ((bitset >> (i)) & 0b1)
    axis_cols_opaque[FACE_DIR_DOWN][z][x] |= bitsetAt(1) << y;
    axis_cols_opaque[FACE_DIR_UP][z][x] |= bitsetAt(0) << y;
    axis_cols_opaque[FACE_DIR_LEFT][y][z] |= bitsetAt(3) << x;
    axis_cols_opaque[FACE_DIR_RIGHT][y][z] |= bitsetAt(2) << x;
    axis_cols_opaque[FACE_DIR_BACK][y][x] |= bitsetAt(4) << z;
    axis_cols_opaque[FACE_DIR_FRONT][y][x] |= bitsetAt(5) << z;
#undef bitsetAt
}

static bool chunkBitmapFindUnsetPosition(ChunkBitmap bitmap,
                                         const Chunk* chunk,
                                         FacesColumns faces_cols,
                                         FacesColumns faces_cols_opaque,
                                         u16* total_blocks_processed,
                                         VECTOR* out_pos) {
    // This alternates iteration between two corners of the chunk
    // working inwards. Allows for slightly higher probability of
    // finding an unset position faster than uni-directional iteration.
    // If there is an efficient way to iterate over 4 corners in
    // opposing diagonals, then it would be good to replace this
    // with that, conditional on keeping bit lookups on an X-axis
    // bitset efficient.
    for (u8 y = 0; y < CHUNK_SIZE; y++) {
        for (u8 z = 0; z < CHUNK_SIZE; z++) {
            const ChunkBitmapBitset x_fwd_bits = bitmap[(y * CHUNK_SIZE) + z];
            const bool fwd_some_set = x_fwd_bits != (CHUNK_SIZE * CHUNK_SIZE) - 1;
            const ChunkBitmapBitset x_bwd_bits = bitmap[((CHUNK_SIZE - 1 - y) * CHUNK_SIZE) + (CHUNK_SIZE - 1 - z)];
            const bool bwd_some_set = x_bwd_bits != (CHUNK_SIZE * CHUNK_SIZE) - 1;
            if (!fwd_some_set && !bwd_some_set) {
                continue;
            }
            const u8 i = y + z;
            if (fwd_some_set && bwd_some_set) {
                for (u8 x = 0; x < CHUNK_SIZE; x++) {
                    VECTOR pos;
                    ChunkBitmapBitset x_bitset;
                    if ((i + x) % 2 != 0) {
                        pos = vec3_i32(
                            CHUNK_SIZE - 1 - x,
                            CHUNK_SIZE - 1 - y,
                            CHUNK_SIZE - 1 - z
                        );
                        x_bitset = x_bwd_bits;
                    } else {
                        pos = vec3_i32(x, y, z);
                        x_bitset = x_fwd_bits;
                    }
                #define updateBitmap(x_bitset, _x, _y, _z, _pos) \
                    if ((((x_bitset) >> (_x)) & 0b1) == 1) { \
                        continue; \
                    } \
                    const IBlock* iblock = chunkGetBlockVec(chunk, &_pos); \
                    const Block* block = VCAST_PTR(Block*, iblock); \
                    if (blockGetOpacityBitset(block->id, block->orientation) != 0b111111) { \
                        *out_pos = _pos; \
                        return true; \
                    } \
                    DEBUG_LOG("Found unset, marking: " VEC_PATTERN "\n", VEC_LAYOUT(pos)); \
                    chunkBitmapSetBit(bitmap, &_pos); \
                    /* Update masks for BGM */ \
                    addVoxelToFaceColumns( \
                        faces_cols, \
                        faces_cols_opaque, \
                        iblock, \
                        (_x) + 1, \
                        (_y) + 1, \
                        (_z) + 1 \
                    ); \
                    (*total_blocks_processed)++
                    updateBitmap(x_bitset, pos.vx, pos.vy, pos.vz, pos);
                }
            }
            u8 x_bitset;
            int start;
            int end;
            int increment;
            if (fwd_some_set) {
                x_bitset = x_fwd_bits;
                start = 0;
                end = CHUNK_SIZE;
                increment = 1;
            } else {
                x_bitset = x_bwd_bits;
                start = CHUNK_SIZE - 1;
                end = -1;
                increment = -1;
            }
            for (int x = start; x != end; x += increment) {
                const VECTOR pos = vec3_i32(x, y, z);
                updateBitmap(x_bitset, x, y, z, pos);
            }
        }
    }
    return false;
    #undef updateBitmap
}

static bool chunkBitmapFindRoot(ChunkBitmap bitmap,
                                const Chunk* chunk,
                                FacesColumns faces_cols,
                                FacesColumns faces_cols_opaque,
                                VECTOR* out_pos) {
    for (u8 y = 0; y < CHUNK_SIZE; y++) {
        for (u8 z = 0; z < CHUNK_SIZE; z++) {
            for (u8 x = 0; x < CHUNK_SIZE; x++) {
                const VECTOR pos = vec3_i32(x, y, z);
                const IBlock* iblock = chunkGetBlockVec(chunk, &pos);
                const Block* block = VCAST_PTR(Block*, iblock);
                if (blockGetType(block->id) != BLOCKTYPE_SOLID) {
                    *out_pos = pos;
                    return true;
                }
                // Update masks for BGM
                addVoxelToFaceColumns(
                    faces_cols,
                    faces_cols_opaque,
                    iblock,
                    x + 1,
                    y + 1,
                    z + 1
                );
                chunkBitmapSetBit(bitmap, &pos);
            }
        }
    }
    return false;
}

static u8 chunkBitmapFillSolid(ChunkBitmap bitmap,
                               const Chunk* chunk,
                               VECTOR starting_pos,
                               FacesColumns faces_cols,
                               FacesColumns faces_cols_opaque) {
    cvector(VECTOR) queue = NULL;
    cvector_init(queue, 0, NULL);
    cvector_push_back(queue, starting_pos);
    chunkBitmapSetBit(bitmap, &starting_pos);
    u8 processed = 0;
    while (cvector_size(queue) > 0) {
        const VECTOR pos = queue[cvector_size(queue) - 1];
        cvector_pop_back(queue);
        processed++;
        const IBlock* iblock = chunkGetBlockVec(chunk, &pos);
        // Update masks for BGM
        addVoxelToFaceColumns(
            faces_cols,
            faces_cols_opaque,
            iblock,
            pos.vx + 1,
            pos.vy + 1,
            pos.vz + 1
        );
        const Block* block = VCAST_PTR(Block*, iblock);
        for (FaceDirection face_direction = FACE_DIR_DOWN; face_direction <= FACE_DIR_FRONT; face_direction++) {
            const VECTOR normal = vec3_i32(
                FACE_DIRECTION_NORMALS[face_direction].vx,
                FACE_DIRECTION_NORMALS[face_direction].vy,
                FACE_DIRECTION_NORMALS[face_direction].vz
            );
            const VECTOR next_pos = vec3_add(pos, normal);
            if (chunkBlockIndexOOB(next_pos.vx, next_pos.vy, next_pos.vz)
                || chunkBitmapGetBit(bitmap, &next_pos) == 1) {
                // OOB or Already visited
                continue;
            }
            const Block* opposing_block = VCAST_PTR(Block*, chunkGetBlockVec(chunk, &next_pos));
            const VECTOR opposing_normal = vec3_i32(
                -normal.vx,
                -normal.vy,
                -normal.vz
            );
            const FaceDirection face_direction = faceDirectionFromNormal(normal);
            const bool facing_opaque = blockIsFaceOpaque(block, face_direction);
            const FaceDirection opposing_face_direction = faceDirectionFromNormal(opposing_normal);
            const bool opposing_opaque = blockIsFaceOpaque(opposing_block, opposing_face_direction);
            const BlockType opposing_block_type = blockGetType(opposing_block->id);
            // TODO: Do we need the blocktype check, or are the dual opaque faces sufficient?
            if (facing_opaque && opposing_opaque && opposing_block_type == BLOCKTYPE_SOLID) {
                cvector_push_back(queue, next_pos);
                chunkBitmapSetBit(bitmap, &next_pos);
            }
        }
    }
    cvector_free(queue);
    return processed;
}

static u8 visitBlock(ChunkBitmap bitmap,
                     const Chunk* chunk,
                     const Block* current_block,
                     // Current position, already marked in bitmap
                     VECTOR pos,
                     const VECTOR normal,
                     cvector(VECTOR)* queue,
                     FacesColumns faces_cols,
                     FacesColumns faces_cols_opaque) {
    const VECTOR next_pos = vec3_add(pos, normal);
    if (chunkBlockIndexOOB(next_pos.vx, next_pos.vy, next_pos.vz)
        || chunkBitmapGetBit(bitmap, &next_pos) == 1) {
        // Outside chunk or already visisted
        return 0;
    }
    const Block* opposing_block = VCAST_PTR(Block*, chunkGetBlockVec(chunk, &next_pos));
    const VECTOR opposing_normal = vec3_i32(
        -normal.vx,
        -normal.vy,
        -normal.vz
    );
    const FaceDirection face_direction = faceDirectionFromNormal(normal);
    const bool facing_opaque = blockIsFaceOpaque(current_block, face_direction);
    const FaceDirection opposing_face_direction = faceDirectionFromNormal(opposing_normal);
    const bool opposing_opaque = blockIsFaceOpaque(opposing_block, opposing_face_direction);
    const BlockType opposing_block_type = blockGetType(opposing_block->id);
    if (opposing_opaque && opposing_block_type == BLOCKTYPE_SOLID) {
        // Solid opaque, light cannot pass through to the next
        // block from this direction
        return chunkBitmapFillSolid(
            bitmap,
            chunk,
            next_pos,
            faces_cols,
            faces_cols_opaque
        );
    } else if (!facing_opaque && !opposing_opaque) {
        // Both transparent, light can traverse "between"
        // these two blocks
        cvector_push_back_safe(queue, next_pos);
        chunkBitmapSetBit(bitmap, &next_pos);
    }
    return 0;
}

static void chunkVisibilityDfsWalkScan(Chunk* chunk,
                                       FacesColumns faces_cols,
                                       FacesColumns faces_cols_opaque) {
    ChunkBitmap bitmap = {0};
    VECTOR root = vec3_i32(0);
    if (!chunkBitmapFindRoot(
        bitmap,
        chunk,
        faces_cols,
        faces_cols_opaque,
        &root
    )) {
        chunk->visibility = 0;
        return;
    }
    cvector(VECTOR) queue = NULL;
    cvector_init(queue, 0, NULL);
    cvector_push_back(queue, root);
    chunkBitmapSetBit(bitmap, &root);
    // We mark every solid block until the first free block
    // in the bitmap, so we start with already having processed
    // those.
    u16 total_blocks_processed = (root.vy * CHUNK_SIZE * CHUNK_SIZE) + (root.vz * CHUNK_SIZE) + root.vx;
    while (total_blocks_processed < CHUNK_DATA_SIZE) {
        u8 visible_sides = 0b000000;
        while (cvector_size(queue) > 0) {
            // This makes this DFS, we save on needing to
            // do a swap with the last element before popping
            // for an efficient pseudo-BFS. So might as well
            // just do DFS instead.
            const VECTOR pos = queue[cvector_size(queue) - 1];
            // DEBUG_LOG("Pos: " VEC_PATTERN "\n", VEC_LAYOUT(pos));
            cvector_pop_back(queue);
            total_blocks_processed++;
            // Update masks for BGM
            addVoxelToFaceColumns(
                faces_cols,
                faces_cols_opaque,
                chunkGetBlockVec(chunk, &pos),
                pos.vx + 1,
                pos.vy + 1,
                pos.vz + 1
            );
            // We assume that when the block position was pushed into the queue,
            // it had already been validated, thus no block properties check here.
            const Block* block = VCAST_PTR(Block*, chunkGetBlock(chunk, pos.vx, pos.vy, pos.vz));
            #define _visitBlock(condition, direction, normal) \
                if (condition) { \
                    visible_sides |= 0b1 << direction; \
                } \
                total_blocks_processed += visitBlock( \
                    bitmap, \
                    chunk, \
                    block, \
                    pos, \
                    normal, \
                    &queue, \
                    faces_cols, \
                    faces_cols_opaque \
                )
            // Left
            _visitBlock(pos.vx == 0, FACE_DIR_LEFT, vec3_i32(-1, 0, 0));
            // Right
            _visitBlock(pos.vx == CHUNK_SIZE - 1, FACE_DIR_RIGHT, vec3_i32(1, 0, 0));
            // Front
            _visitBlock(pos.vz == 0, FACE_DIR_FRONT, vec3_i32(0, 0, -1));
            // Back
            _visitBlock(pos.vz == CHUNK_SIZE - 1, FACE_DIR_BACK, vec3_i32(0, 0, 1));
            // Down
            _visitBlock(pos.vy == 0, FACE_DIR_DOWN, vec3_i32(0, -1, 0));
            // Up
            _visitBlock(pos.vy == CHUNK_SIZE - 1, FACE_DIR_UP, vec3_i32(0, 1, 0));
            #undef _visitBlock
        }
        if (!isPowerOf2(visible_sides)) {
            // Power of 2 implies a single bit is set,
            // thus only one side in the visibility set
            // so that side cannot see another.
            for (u8 i = 0; i < 6; i++) {
                if ((visible_sides & (0b1 << i)) == 0) {
                    continue;
                }
                for (u8 j = i + 1; j < 6; j++) {
                    if ((visible_sides & (0b1 << j)) == 0) {
                        continue;
                    }
                    chunkVisibilitySetBit(&chunk->visibility, i, j);
                }
            }
        }
        VECTOR pos = vec3_i32(0);
        if (!chunkBitmapFindUnsetPosition(
            bitmap,
            chunk,
            faces_cols,
            faces_cols_opaque,
            &total_blocks_processed,
            &pos
        )) {
            break;
        }
        cvector_push_back(queue, pos);
        chunkBitmapSetBit(bitmap, &pos);
    }
    cvector_free(queue);
    DEBUG_LOG(
        "[Chunk] Blocks processed: %d/%d\n",
        total_blocks_processed,
        CHUNK_DATA_SIZE
    );
    // assert(total_blocks_processed == CHUNK_DATA_SIZE);
}

#undef chunkBitmapGetBit
#undef chunkBitmapSetBit

void binaryGreedyMesherBuildMesh(Chunk* chunk, const BreakingState* breaking_state) {
    if (chunk->solid_block_count == 0) {
        // Short circuit this entire process since we don't need to
        // mesh an empty chunk and it is guaranteed to have uniform
        // visibility on all sides.
        chunk->visibility = 0b111111111111111;
        return;
    }
    FacesColumns faces_cols = {0};
    // See binary_greedy_mesher_transparency.md
    FacesColumns faces_cols_opaque = {0};
    FacesColumns col_face_masks = {0};
    chunkVisibilityDfsWalkScan(
        chunk,
        faces_cols,
        faces_cols_opaque
    );
    DEBUG_LOG("[Chunk] Visibility: " INT16_BIN_PATTERN "\n", INT16_BIN_LAYOUT(chunk->visibility));
    // Inner chunk blocks
    /*for (u32 z = 0; z < CHUNK_SIZE; z++) {*/
    /*    for (u32 x = 0; x < CHUNK_SIZE; x++) {*/
    /*        for (u32 y = 0; y < CHUNK_SIZE; y++) {*/
    /*            const IBlock* iblock = chunkGetBlock(chunk, x, y, z);*/
    /*            addVoxelToFaceColumns(*/
    /*                faces_cols,*/
    /*                faces_cols_opaque,*/
    /*                iblock,*/
    /*                x + 1,*/
    /*                y + 1,*/
    /*                z + 1*/
    /*            );*/
    /*        }*/
    /*    }*/
    /*}*/
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
                        .block = vec3_i32(0),
                        .chunk = chunk->position
                    };
                    ChunkBlockPosition light_cb_pos = {
                        .block = vec3_i32(0),
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
                        return;
                    }
                    const Block* block = VCAST_PTR(Block*, current_block);
                    if (blockGetType(block->id) == BLOCKTYPE_EMPTY) {
                        errorAbort(
                            "[BINARY GREEDY MESHER] Empty block encountered while constructing mask [Face: %d] [Chunk: " VEC_PATTERN "] [Block: " VEC_PATTERN "]\n",
                            face,
                            VEC_LAYOUT(chunk->position),
                            VEC_LAYOUT(chunk_block_position.block)
                        );
                        return;
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
    if (breaking_state != NULL && breaking_state->block != NULL) {
        binaryGreedyMesherConstructBreakingOverlay(chunk, breaking_state);
    }
}

static MeshPrimitive* createPrimitive(ChunkMesh* mesh,
                                      const PlaneMeshingDataKey* data,
                                      const u32 width,
                                      const u32 height,
                                      const Texture* texture_override,
                                      const TextureAttributes texture_attributes_override[FACE_DIRECTION_COUNT]) {
    Mesh* face_dir_mesh = &mesh->face_meshes[data->face];
    cvector(MeshPrimitive) prims = face_dir_mesh->p_prims;
    cvector_push_back(prims, (MeshPrimitive){0});
    // cvector can be reallocated by pushing a
    // new element, thus we should re-set it to
    // the mesh object that owns it.
    face_dir_mesh->p_prims = prims;
    MeshPrimitive* primitive = &prims[face_dir_mesh->n_prims++];
    primitive->type = MESH_PRIM_TYPE_QUAD;
    const Texture* texture = texture_override != NULL
        ? texture_override
        : &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    primitive->tpage = texture->tpage;
    primitive->clut = texture->clut;
    const TextureAttributes* attributes = texture_attributes_override != NULL
        ? &texture_attributes_override[data->face]
        : &blockGetFaceAttributes(data->block->id, data->block->metadata_id)[data->face];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = BLOCK_TEXTURE_SIZE * width;
    primitive->tv1 = BLOCK_TEXTURE_SIZE * height;
    const CVECTOR tint = attributes->tint.cd
        ? attributes->tint
        : vec3_rgb(0xFF, 0xFF, 0xFF);
    primitive->r = tint.r;
    primitive->g = tint.g;
    primitive->b = tint.b;
    primitive->light_level = data->light_level;
    return primitive;
}

#define nextRenderAttribute(mesh, attribute_field, index_field, count_field) ({ \
        cvector_push_back((mesh)->attribute_field, (SVECTOR){0}); \
        primitive->index_field = (mesh)->count_field; \
        (&cvector_begin((mesh)->attribute_field)[(mesh)->count_field++]); \
})

static const INDEX INDICES[FACE_DIRECTION_COUNT] = {
    [0]={3,2,1,0},
    [1]={1,0,3,2},
    [2]={3,2,1,0},
    [3]={2,3,0,1},
    [4]={2,3,0,1},
    [5]={3,2,1,0}
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
    // TODO: This should create sets of vertices based on the
    //       block type (implying the model). It will need to
    //       be able to create more than 4 vertices for some
    //       such as stairs.
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
    // TODO: When supporting general block models based on the type
    //       this will need to generate potentially multiple normals
    //       for each set of vertices that make up a face. It will
    //       also need to address models that are not axially aligned
    //       such as sugar cane.
    SVECTOR* norm = nextRenderAttribute(&mesh->face_meshes[face_dir], p_norms, n0, n_norms);
#undef nextRenderAttribute
    *norm = vec3_const_mul(
        FACE_DIRECTION_NORMALS[face_dir],
        ONE
    );
}

static void createMesh(Chunk* chunk,
                       const PlaneMeshingDataKey* data,
                       const Texture* texture_override,
                       const TextureAttributes* texture_attributes_override,
                       const u32 x,
                       const u32 y,
                       const u32 w,
                       const u32 h) {
    MeshPrimitive* primitive = createPrimitive(
        &chunk->mesh,
        data,
        w,
        h,
        texture_override,
        texture_attributes_override
    );
    createVertices(
        chunk,
        primitive,
        data->face,
        data->axis,
        x,
        y,
        w,
        h
    );
    createNormal(
        &chunk->mesh,
        primitive,
        data->face
    );
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

void binaryGreedyMesherConstructBreakingOverlay(Chunk* chunk, const BreakingState* breaking_state) {
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(
        &breaking_state->position,
        CHUNK_SIZE
    );
    const u8 visible_sides_bitset = breaking_state->visible_sides_bitset;
    const Block* block = VCAST_PTR(Block*, breaking_state->block);
    TextureAttributes texture_attributes[FACE_DIRECTION_COUNT] = {0};
    /*#pragma GCC unroll 6*/
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
        attributes->tint = blockGetFaceAttributes(block->id, block->metadata_id)[face_dir].tint;
        const Texture texture = (Texture) {
            .tpage = getTPage(
                2,
                0,
                breaking_texture_offscreen.x,
                breaking_texture_offscreen.y
            ),
            .clut = textures[ASSET_TEXTURE__STATIC__TERRAIN].clut
        };
        u32 x = 0;
        u32 y = 0;
        u32 axis = 0;
        #define pos(_axis, _x, _y) \
            axis = chunk_block_position.block.v##_axis; \
            x = chunk_block_position.block.v##_x; \
            y = chunk_block_position.block.v##_y
        ChunkBlockPosition light_cb_pos = (ChunkBlockPosition) {
            .block = vec3_i32(0),
            .chunk = chunk->position
        };
        #define lightPos(_x, _y, _z) \
            light_cb_pos.block.vx = _x; \
            light_cb_pos.block.vy = _y; \
            light_cb_pos.block.vz = _z
        switch (current_face_dir) {
            case FACE_DIR_DOWN:
                pos(y, x, z);
                lightPos(x, axis - 1, y);
                break;
            case FACE_DIR_UP:
                pos(y, x, z);
                lightPos(x, axis + 1, y);
                break;
            case FACE_DIR_LEFT:
                pos(x, z, y);
                lightPos(axis - 1, y, x);
                break;
            case FACE_DIR_RIGHT:
                pos(x, z, y);
                lightPos(axis + 1, y, x);
                break;
            case FACE_DIR_BACK:
                pos(z, x, y);
                lightPos(x, y, axis - 1);
                break;
            case FACE_DIR_FRONT:
                pos(z, x, y);
                lightPos(x, y, axis + 1);
                break;
        }
        const VECTOR light_query_pos = chunkBlockToWorldPosition(&light_cb_pos, CHUNK_SIZE);
        const PlaneMeshingDataKey key = (PlaneMeshingDataKey) {
            .face = current_face_dir,
            // This should be the coordinate value for which ever axis stays constant
            // in the facing direction, i.e. in the UP direction the Y coord doesn't
            // change at each vertex, that would be the value here.
            .axis = axis,
            .light_level= worldGetLightValue(chunk->world, &light_query_pos),
            .block = block
        };
        createMesh(
            chunk,
            &key,
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
