#include "chunk.h"

#include <cube.h>
#include <inline_c.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../../../logging/logging.h"
#include "../../../math/math_utils.h"
#include "../../../math/vector.h"
#include "../../../structure/cvector.h"
#include "../../../structure/cvector_utils.h"
#include "../../../structure/primitive/direction.h"
#include "../../../util/interface99_extensions.h"
#include "../../items/items.h"
#include "../generation/noise.h"
#include "chunk_mesh.h"
#include "chunk_structure.h"
#include "meshing/binary_greedy_mesher.h"
#include "psxapi.h"

const LightUpdateLimits chunk_light_update_limits = (LightUpdateLimits) {
    .add_block = CHUNK_LIGHT_ADD_BLOCK_UPDATES_PER_TICK,
    .add_sky = CHUNK_LIGHT_ADD_SKY_UPDATES_PER_TICK,
    .remove_block = CHUNK_LIGHT_REMOVE_BLOCK_UPDATES_PER_TICK,
    .remove_sky = CHUNK_LIGHT_REMOVE_SKY_UPDATES_PER_TICK
};

// Forward declaration
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);
FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
FWD_DECL Chunk* worldGetChunkFromChunkBlock(const World* world, const ChunkBlockPosition* position);
FWD_DECL LightLevel worldGetLightType(const World* world,
                                      const VECTOR* position,
                                      const LightType light_type);
FWD_DECL void worldSetLightValue(const World* world,
                                 const VECTOR* position,
                                 LightLevel light_value,
                                 const LightType light_type);
FWD_DECL void worldSetLightValueChunkBlock(const World* world,
                                           const ChunkBlockPosition* position,
                                           u8 light_value,
                                           const LightType light_type);
FWD_DECL LightLevel worldGetLightTypeChunkBlock(const World* world,
                                                const ChunkBlockPosition* position,
                                                const LightType light_type);
FWD_DECL void worldRemoveLightType(const World* world,
                                   const VECTOR* position,
                                   const LightType light_type);
FWD_DECL void worldRemoveLightTypeChunkBlock(const World* world,
                                             const ChunkBlockPosition* position,
                                             const LightType light_type);
FWD_DECL LightLevel worldGetInternalLightLevel(const World* world);

void chunkDestroyDroppedItem(void* elem) {
    IItem** iitem = (IItem**) elem;
    if (iitem == NULL || *iitem == NULL) {
        return;
    }
    VCALL(**iitem, destroy);
    itemDestroy(*iitem);
}

static u64 lightAddNodeDataHash(const void* item, u64 seed0, u64 seed1) {
    const LightAddNode* node = item;
    return hashmap_xxhash3(
        &node->position,
        sizeof(VECTOR),
        seed0,
        seed1
    );
}

static u64 lightRemoveNodeDataHash(const void* item, u64 seed0, u64 seed1) {
    const LightRemoveNode* node = item;
    return hashmap_xxhash3(
        &node->position,
        sizeof(VECTOR),
        seed0,
        seed1
    );
}

static int lightAddNodeCompare(const void* a, const void* b, void* ignored) {
    const LightAddNode* node_a = a;
    const LightAddNode* node_b = a;
    // Negation here since this compare function is like the
    // cmp(..) function in the standard library, where a return
    // value of 0 implies equivalence.
    return !vec3_equal(node_a->position, node_b->position);
}

static int lightRemoveNodeCompare(const void* a, const void* b, void* ignored) {
    const LightRemoveNode* node_a = a;
    const LightRemoveNode* node_b = a;
    // Negation here since this compare function is like the
    // cmp(..) function in the standard library, where a return
    // value of 0 implies equivalence.
    return !vec3_equal(node_a->position, node_b->position);
}

void chunkInit(Chunk* chunk) {
    chunk->is_top = true;
    chunk->lightmap_updated = false;
    chunk->mesh_updated = false;
    chunk->dropped_items = NULL;
    cvector_init(chunk->dropped_items, 0, chunkDestroyDroppedItem);
    chunk->updates.sunlight_add_queue = hashmap_new(
        sizeof(LightAddNode),
        1,
        0,
        0,
        lightAddNodeDataHash,
        lightAddNodeCompare,
        NULL,
        NULL
    );
    chunk->updates.light_add_queue = hashmap_new(
        sizeof(LightAddNode),
        1,
        0,
        0,
        lightAddNodeDataHash,
        lightAddNodeCompare,
        NULL,
        NULL
    );
    chunk->updates.light_remove_queue = hashmap_new(
        sizeof(LightRemoveNode),
        1,
        0,
        0,
        lightRemoveNodeDataHash,
        lightRemoveNodeCompare,
        NULL,
        NULL
    );
    chunk->updates.sunlight_remove_queue = hashmap_new(
        sizeof(LightRemoveNode),
        1,
        0,
        0,
        lightRemoveNodeDataHash,
        lightRemoveNodeCompare,
        NULL,
        NULL
    );
    memset(
        chunk->lightmap,
        0,
        sizeof(*chunk->lightmap) * CHUNK_DATA_SIZE
    );
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
}

void chunkDestroy(const Chunk* chunk) {
    chunkMeshDestroy(&chunk->mesh);
    cvector_free(chunk->dropped_items);
    hashmap_free(chunk->updates.sunlight_add_queue);
    hashmap_free(chunk->updates.light_add_queue);
    hashmap_free(chunk->updates.sunlight_remove_queue);
    hashmap_free(chunk->updates.light_remove_queue);
}

void chunkGenerate3DHeightMap(Chunk* chunk, const VECTOR* position) {
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 y = 0; y < CHUNK_SIZE; y++) {
            for (i32 z = 0; z < CHUNK_SIZE; z++) {
                const int height = noise3d(
                    x + position->vx,
                    y + position->vy,
                    z + position->vz
                );
                chunk->blocks[chunkBlockIndex(x, y, z)] = height >= 0
                    ? airBlockCreate(NULL)
                    : stoneBlockCreate(NULL);
            }
        }
    }
}

void chunkClearMesh(Chunk* chunk) {
    chunkMeshClear(&chunk->mesh);
}

INLINE static void chunkGenerateMeshWithBreakingState(Chunk* chunk, const BreakingState* breaking_state) {
    binaryGreedyMesherBuildMesh(chunk, breaking_state);
    chunk->mesh_updated = true;
}

void chunkGenerateMesh(Chunk* chunk) {
    chunkGenerateMeshWithBreakingState(chunk, NULL);
}

#define getSunlight(gen_ctx, x, z) (gen_ctx)->sunlight_heightmap[((x) * CHUNK_SIZE) + (z)]

void chunkGenerateLightmap(Chunk* chunk, ChunkGenerationContext* gen_ctx) {
    bool is_top = true;
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            for (i32 y = CHUNK_SIZE - 1; y >= 0; y--) {
                const VECTOR position = vec3_i32(x, y, z);
                const IBlock* iblock = chunkGetBlockVec(chunk, &position);
                assert(iblock != NULL);
                const Block* block = VCAST_PTR(Block*, iblock);
                if (blockCanPropagateSunlight(block->id)
                    && !blockIsFaceOpaque(block, FACE_DIR_UP)) {
                    // Note we don't queue updates here, instead just set
                    // the lightmap value directly since we determine the
                    // horizontal propagation based on a sunlight columns
                    // that are next to no sunlight columns later.
                    lightMapSetValue(
                        chunk->lightmap,
                        position,
                        15,
                        LIGHT_TYPE_SKY
                    );
                    getSunlight(gen_ctx, x, z)--;
                } else {
                    // NOTE: We are at the top so long as there is at least
                    // one air block at the top of the chunk (for now). In
                    // future this will be based on whether the chunk Y range
                    // is within the heighmap for this column of chunks.
                    is_top &= y == CHUNK_SIZE - 1;
                    break;
                }
            }
        }
    }
    // NOTE: This will change, being moved into the chunk generator instead
    chunk->is_top = is_top;
}

void chunkPropagateLightmap(Chunk* chunk, ChunkGenerationContext* gen_ctx) {
    if (!chunk->is_top) {
        return;
    }
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            // We only want to determine if the sunlight
            // columns have visible neighbours that are
            // not in sunlight, so we skip any blocks that
            // are not in the sunlight columns.
            const i32 sunlight_col_end = getSunlight(gen_ctx, x, z);
            for (i32 y = CHUNK_SIZE - 1; y >= sunlight_col_end; y--) {
                const VECTOR position = vec3_i32(x, y, z);
                // Left, right, back, front
                for (FaceDirection face_dir = FACE_DIR_LEFT; face_dir <= FACE_DIR_FRONT; face_dir++) {
                    const VECTOR face_position = vec3_add(
                        vec3_i32(x, y, z),
                        WORLD_FACE_DIRECTION_NORMALS[face_dir]
                    );
                    ChunkBlockPosition cb_pos = (ChunkBlockPosition) {
                        .chunk = chunk->position,
                        .block = face_position
                    };
                    const VECTOR query_pos = chunkBlockToWorldPosition(
                        &cb_pos,
                        CHUNK_SIZE
                    );
                    cb_pos = worldToChunkBlockPosition(&query_pos, CHUNK_SIZE);
                    const Chunk* query_chunk = worldGetChunkFromChunkBlock(
                        chunk->world,
                        &cb_pos
                    );
                    if (query_chunk == NULL || getSunlight(gen_ctx, cb_pos.block.vx, cb_pos.block.vz) <= y) {
                        // Highest sunlight-propagating block in facing direction is lower
                        // than this block, so it's just sunlight. We don't need to propagate
                        // sunlight there since we already did that.
                        continue;
                    }
                    // Block in the facing direction is below the sunlight column, so
                    // can potentially propagate skylight. We check that to determine
                    // whether propagation is necessary.
                    const IBlock* iblock = chunkGetBlockVec(query_chunk, &cb_pos.block);
                    assert(iblock != NULL);
                    const Block* block = VCAST_PTR(Block*, iblock);
                    if (blockCanPropagateSunlight(block->id)
                        && !blockIsFaceOpaque(block, faceDirectionOpposing(face_dir))) {
                        // Light can propagate in this direction, so we have at
                        // least one valid direction. Add this block to the queue
                        // and skip the rest of the direction checks here.
                        chunkSetLightValue(
                            chunk,
                            &position,
                            15,
                            LIGHT_TYPE_SKY
                        );
                        break;
                    }
                } 
            }
        }
    }
}

#undef getSunlight

static void chunkRenderDroppedItems(const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    IItem** item;
    cvector_for_each_in(item, chunk->dropped_items) {
        if (*item == NULL) {
            continue;
        }
        VCALL_SUPER(**item, Renderable, renderWorld, chunk, ctx, transforms);
    }
}

static bool chunkIsOutsideFrustum(const Chunk* chunk, const Frustum* frustum, const Transforms* transforms) {
    const AABB aabb = (AABB) {
        .min = vec3_i32(
            (chunk->position.vx * CHUNK_BLOCK_SIZE) << FIXED_POINT_SHIFT,
            (-chunk->position.vy * CHUNK_BLOCK_SIZE) << FIXED_POINT_SHIFT,
            (chunk->position.vz * CHUNK_BLOCK_SIZE) << FIXED_POINT_SHIFT
        ),
        .max = vec3_i32(
            ((chunk->position.vx + 1) * CHUNK_BLOCK_SIZE) << FIXED_POINT_SHIFT,
            (-(chunk->position.vx + 1) * CHUNK_BLOCK_SIZE) << FIXED_POINT_SHIFT,
            ((chunk->position.vx + 1) * CHUNK_BLOCK_SIZE) << FIXED_POINT_SHIFT
        )
    };
    const FrustumQueryResult result = frustumContainsAABB(frustum, &aabb);
    return result == FRUSTUM_OUTSIDE;
}

void chunkRender(Chunk* chunk,
                 RenderContext* ctx,
                 Transforms* transforms) {
    // if (chunkIsOutsideFrustum(chunk, &ctx->camera->frustum, transforms)) {
    //     DEBUG_LOG("[CHUNK " VEC_PATTERN "] Not visible\n", VEC_LAYOUT(chunk->position));
    //     // return;
    // } else {
    //     DEBUG_LOG("[CHUNK " VEC_PATTERN "] Visible\n", VEC_LAYOUT(chunk->position));
    // }
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&VEC3_I16_ZERO, &omtx);
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
    chunkMeshRender(
        &chunk->mesh,
        worldGetInternalLightLevel(chunk->world),
        ctx,
        transforms
    );
    // Restore matrix
    PopMatrix();
    chunkRenderDroppedItems(chunk, ctx, transforms);
}

#define checkIndexOOB(x, y, z) ((x) >= CHUNK_SIZE || (x) < 0 \
	|| (y) >= CHUNK_SIZE || (y) < 0 \
	|| (z) >= CHUNK_SIZE || (z) < 0)

IBlock* chunkGetBlock(const Chunk* chunk, const i32 x, const i32 y, const i32 z) {
    if (checkIndexOOB(x, y, z)) {
        return airBlockCreate(NULL);
    }
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return airBlockCreate(NULL);
    }
    return chunk->blocks[chunkBlockIndex(
        position->vx,
        position->vy,
        position->vz
    )];
}

#define HALF_BLOCK_SIZE (BLOCK_SIZE >> 1)

static void applyItemWorldState(const Chunk* chunk,
                                Item* item,
                                const VECTOR* block_position) {
    // Construct vertices relative to chunk mesh bottom left origin
    const i32 chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    const i32 chunk_origin_y = chunk->position.vy * CHUNK_SIZE;
    const i32 chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
    // Mark the item as in world and create physics object +
    // entity structures
    itemSetWorldState(item, true);
    const VECTOR item_position = vec3_const_lshift(
        vec3_i32(
            (chunk_origin_x + block_position->vx) * BLOCK_SIZE + HALF_BLOCK_SIZE,
            (chunk_origin_y + block_position->vy) * BLOCK_SIZE + HALF_BLOCK_SIZE,
            (chunk_origin_z + block_position->vz) * BLOCK_SIZE + HALF_BLOCK_SIZE
        ),
        FIXED_POINT_SHIFT
    );
    /*const VECTOR item_position = vec3_i32(*/
    /*    0,*/
    /*    2867200,*/
    /*    0*/
    /*);*/
    iPhysicsObjectSetPosition(
        item->world_physics_object,
        &item_position
    );
    item->world_physics_object->rotation.yaw = rand() % 32768;
    item->world_physics_object->rotation.pitch  = rand() % 32768;
    item->world_physics_object->move.forward = positiveModulo(rand(), (ONE * 2)) - ONE;
    /*item->world_physics_object->move.strafe = positiveModulo(rand(), (ONE * 2)) - ONE;*/
}

static LightLevel inferSunlightValueFromNeighbours(const Chunk* chunk,
                                                   const VECTOR* position) {
    ChunkBlockPosition cb_pos = (ChunkBlockPosition) {
        .chunk = chunk->position,
        .block = vec3_i32_all(0)
    };
    LightLevel current_max = 0;
    #pragma GCC unroll 6
    for (FaceDirection face_dir = FACE_DIR_DOWN; face_dir <= FACE_DIR_FRONT; face_dir++) {
        cb_pos.block = vec3_add(
            *position,
            WORLD_FACE_DIRECTION_NORMALS[face_dir]
        );
        const LightLevel neighbour_light_level = worldGetLightTypeChunkBlock(
            chunk->world,
            &cb_pos,
            LIGHT_TYPE_SKY
        );
        if (face_dir == FACE_DIR_UP && neighbour_light_level == 15) {
            return 15;
        }
        current_max = max(current_max, neighbour_light_level);
    }
    return max(0, current_max - 1);
}

INLINE static int modifyVoxel0(Chunk* chunk,
                               const VECTOR* position,
                               const Block* new_block,
                               const bool drop_item,
                               IItem** item_result) {
    const i32 x = position->vx;
    const i32 y = position->vy;
    const i32 z = position->vz;
    if (checkIndexOOB(x, y, z)) {
        return 2;
    }
    const IBlock* old_iblock = chunk->blocks[chunkBlockIndex(x, y, z)];
    const Block* old_block = VCAST_PTR(Block*, old_iblock);
    const bool old_is_air = old_block->id == BLOCKID_AIR;
    if (old_block->light_level > 0 && new_block->light_level == 0) {
        chunkRemoveLightValue(
            chunk,
            position,
            LIGHT_TYPE_BLOCK
        );
    } else {
        chunkSetLightValue(
            chunk,
            position,
            new_block->light_level,
            LIGHT_TYPE_BLOCK
        );
    }
    if (blockCanPropagateSunlight(new_block->id)) {
        const LightLevel new_light_level = inferSunlightValueFromNeighbours(
            chunk,
            position
        );
        chunkSetLightValue(
            chunk,
            position,
            new_light_level,
            LIGHT_TYPE_SKY
        );
    } else {
        chunkRemoveLightValue(
            chunk,
            position,
            LIGHT_TYPE_SKY
        );
    }
    IItem* iitem = VCALL(*old_iblock, destroy, drop_item);
    if (iitem != NULL && iitem->self != NULL) {
        cvector_push_back(
            chunk->dropped_items,
            iitem
        );
        Item* item = VCAST(Item*, *iitem);
        applyItemWorldState(chunk, item, position);
        if (item_result != NULL) {
            *item_result = iitem;
        }
    } else if (item_result != NULL) {
        *item_result = NULL;
    }
    return !old_is_air;
}

bool chunkModifyVoxel(Chunk* chunk,
                      const VECTOR* position,
                      IBlock* iblock,
                      const bool drop_item,
                      IItem** item_result) {
    const Block* block = VCAST_PTR(Block*, iblock);
    const int result = modifyVoxel0(
        chunk,
        position,
        block,
        drop_item,
        item_result
    );
    if (result == 2) {
        return false;
    }
    chunk->blocks[chunkBlockIndex(
        position->vx,
        position->vy,
        position->vz
    )] = iblock;
    chunk->mesh_updated = true;
    return true;
}

IBlock* chunkModifyVoxelConstructed(Chunk* chunk,
                                    const VECTOR* position,
                                    const BlockConstructor block_constructor,
                                    IItem* from_item,
                                    const bool drop_item,
                                    IItem** item_result) {
    IBlock* iblock = block_constructor(from_item);
    const Block* block = VCAST_PTR(Block*, iblock);
    const int result = modifyVoxel0(
        chunk,
        position,
        block,
        drop_item,
        item_result
    );
    if (result == 2) {
        VCALL(*iblock, destroy, true);
        return NULL;
    }
    IBlock* return_block = chunk->blocks[chunkBlockIndex(
        position->vx,
        position->vy,
        position->vz
    )] = iblock;
    chunk->mesh_updated = true;
    return return_block;
}

bool itemPickupValidator(const Item* item, void* ctx) {
    const Inventory* inventory = (Inventory*) ctx;
    // 1. Does the item already exist in the inventory?
    //   a. [1:TRUE] Does the existing have space?
    //     i. [a:TRUE] Return true
    //     ii. [a:FALSE] Go to 2
    //   b. [1:FALSE] Go to 2
    // 2. Is there space in the inventory
    //   a. [2:TRUE] Return true
    //   b. [2:FALSE] Return false
    u8 from_slot = INVENTORY_SLOT_STORAGE_OFFSET;
    u8 next_free = INVENTORY_NO_FREE_SLOT;
    while (true) {
        const Slot* slot = inventorySearchItem(inventory, item->id, from_slot, &next_free);
        if (slot == NULL) {
            if (next_free == INVENTORY_NO_FREE_SLOT) {
                return false;
            }
            slot = &inventory->slots[next_free];
            if (inventorySlotGetItem(slot) == NULL) {
                break;
            }
        }
        const Item* slot_item = VCAST(Item*, *inventorySlotGetItem(slot));
        const int stack_left = itemGetMaxStackSize(slot_item->id) - slot_item->stack_size;
        if (stack_left != 0) {
            break;
        }
        from_slot = slot->index + 1;
        next_free = INVENTORY_NO_FREE_SLOT;
    }
    return true;
}

void updateItemChunkOwnership(const Chunk* chunk,
                              IItem* iitem,
                              const u32 index) {
    // NOTE: This could be callback invoked from the PhysicsObject
    //       when there is movement as opposed to an explicit check
    //       here every time we update an item. The tradeoff there
    //       is that we need to maintain context in memory for every
    //       item that exists to pass to a callback registered when
    //       creating the item. That adds unnecessary memory overhead
    //       and extra complexity (headers + linkage wise) to the
    //       item definition that just isn't worth it. So this exists
    //       to do the update check every time, checking the velocity
    //       to determine if the item has moved on each update cycle
    //       (as a precondition for checking the chunk ownership) is
    //       substantially cheaper.
    const Item* item = VCAST_PTR(Item*, iitem);
    if (!vec3_equal(item->world_physics_object->velocity, VEC3_I32_ZERO)) {
        // No velocity implies no movement, so we can't have changed
        // chunks since we last checked
        return;
    }
    const VECTOR item_world_pos = vec3_const_div(
        item->world_physics_object->position,
        ONE_BLOCK
    );
    const ChunkBlockPosition cb_pos = worldToChunkBlockPosition(
        &item_world_pos,
        CHUNK_SIZE
    );
    if (vec3_equal(chunk->position, cb_pos.chunk)) {
        // Still in same chunk, don't do anything
        return;
    }
    // Has moved to different chunk
    cvector_erase(chunk->dropped_items, index);
    Chunk* new_chunk = worldGetChunkFromChunkBlock(chunk->world, &cb_pos);
    cvector_push_back(new_chunk->dropped_items, iitem);
}

void chunkUpdate(Chunk* chunk, const Player* player, BreakingState* breaking_state) {
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    // We are using chunk relative coords in absolute units and not in
    // fixed point format since item positons only need to be relatively
    // accurate not exact so we can save on the extra caclulation overhead
    // of needing to use division for worldspace accuracy.
    // @see itemPickupValidator
    const VECTOR pos = vec3_i32(
        player->physics_object.position.vx >> FIXED_POINT_SHIFT,
        -player->physics_object.position.vy >> FIXED_POINT_SHIFT,
        player->physics_object.position.vz >> FIXED_POINT_SHIFT
    );
    
    for (u32 i = 0; i < cvector_size(chunk->dropped_items);) {
        IItem* iitem = chunk->dropped_items[i];
        if (iitem == NULL) {
            i++;
            continue;
        }
        Item* item = VCAST(Item*, *iitem);
        if (itemUpdate(item,
                       chunk->world,
                       &pos,
                       inventory,
                       itemPickupValidator)) {
            const InventoryStoreResult result = inventoryStoreItem(inventory, iitem);
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
                    VCALL(*iitem, destroy);
                    itemDestroy(iitem);
                case INVENTORY_STORE_RESULT_ADDED_NEW_SLOT:
                    // We reuse this item instance as the inventory instance now so don't
                    // free it, just remove the array entry (no element destructor set
                    // on the cvector instance)
                    cvector_erase(chunk->dropped_items, i);
                    continue;
                    break;
            }
        }
        updateItemChunkOwnership(chunk, iitem, i);
        i++;
    }
    chunkUpdateLight(chunk, chunk_light_update_limits);
    if (breaking_state != NULL) {
        if (breaking_state->chunk_remesh_trigger) {
            // (In meshing) if the block matches the breaking state ensure it has unique mesh primitives
            // with correct tpage and texture window into the render target
            chunkClearMesh(chunk);
            chunkGenerateMeshWithBreakingState(chunk, breaking_state);
            breaking_state->chunk_remesh_trigger = false;
            chunk->mesh_updated = false;
            chunk->lightmap_updated = false;
        }
        if (breaking_state->reset_trigger) {
            chunkClearMesh(chunk);
            chunkGenerateMesh(chunk);
            breakingStateReset(*breaking_state);
            chunk->mesh_updated = false;
            chunk->lightmap_updated = false;
        }
    } else if (chunk->mesh_updated || chunk->lightmap_updated) {
        chunkClearMesh(chunk);
        chunkGenerateMesh(chunk);
        chunk->mesh_updated = false;
        chunk->lightmap_updated = false;
    }
}

LightLevel chunkGetLightValue(const Chunk* chunk,
                              const VECTOR* position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return createLightLevel(0, worldGetInternalLightLevel(chunk->world));
    }
    return lightMapGetValue(chunk->lightmap, *position);
}

LightLevel chunkGetLightType(const Chunk* chunk,
                             const VECTOR* position,
                             const LightType light_type) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return createLightLevel(0, worldGetInternalLightLevel(chunk->world));
    }
    return lightMapGetType(chunk->lightmap, *position, light_type);
}

void chunkSetLightValue(Chunk* chunk,
                        const VECTOR* position,
                        const LightLevel light_value,
                        const LightType light_type) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return;
    }
    lightMapSetValue(
        chunk->lightmap,
        *position,
        light_value,
        light_type
    );
    // Switch explicitly here instead of just using a ternary to get the
    // queue pointer since we may increase the size of the queue which
    // will mean the pointer is updated and thus we need to set the
    // value on the struct, also because additional light types (not likely)
    // should have compile-time failures for places they should be used.
    HashMap* queue = NULL;
    switch (light_type) {
        case LIGHT_TYPE_BLOCK:
            queue = chunk->updates.light_add_queue;
            break;
        case LIGHT_TYPE_SKY:
            queue = chunk->updates.sunlight_add_queue;
            break;
    }
    assert(queue != NULL);
    const LightAddNode node = (LightAddNode) {
        *position,
        chunk
    };
    hashmap_set(queue, &node);
    if (hashmap_oom(queue)) {
        errorAbort("[CHUNK] Failed to enqueue light add update, hashmap OOM\n");
    }
}

void chunkRemoveLightValue(Chunk* chunk,
                           const VECTOR* position,
                           const LightType light_type) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return;
    }
    const LightLevel light_value = lightMapGetType(
        chunk->lightmap,
        *position,
        light_type
    );
    const LightRemoveNode node = (LightRemoveNode) {
        *position,
        chunk,
        light_value
    };
    HashMap* queue = NULL;
    switch (light_type) {
        case LIGHT_TYPE_BLOCK:
            queue = chunk->updates.light_remove_queue;
            break;
        case LIGHT_TYPE_SKY:
            queue = chunk->updates.sunlight_remove_queue;
            break;
    }
    hashmap_set(queue,  &node);
    if (hashmap_oom(queue)) {
        errorAbort("[CHUNK] Failed to enqueue light remove update, hashmap OOM");
    }
    lightMapSetValue(
        chunk->lightmap,
        *position,
        0,
        light_type
    );
}

void chunkUpdateLight(Chunk* chunk, const LightUpdateLimits limits) {
    chunkUpdateRemoveLight(chunk, limits);
    chunkUpdateAddLight(chunk, limits);
}

#define lightCheckLimit(limit, iter) ((limit) < 0 || (iter) < ((size_t) limit))

void chunkUpdateAddLight(Chunk* chunk, const LightUpdateLimits limits) {
    chunk->lightmap_updated = hashmap_count(chunk->updates.light_add_queue) != 0
                            || hashmap_count(chunk->updates.sunlight_add_queue) != 0;
    size_t processed_updates = 0;
    size_t iter = 0;
    void* item;
    while (lightCheckLimit(limits.add_block, processed_updates)
            && hashmap_iter(chunk->updates.light_add_queue, &iter, &item)) {
        const LightAddNode* node = item;
        const VECTOR current_pos = node->position;
        const Chunk* current_chunk = node->chunk;
        hashmap_delete(chunk->updates.light_add_queue, node);
        // Need to reset cursor as mandated by hashmap_iter docstring.
        // Doesn't impact this loop since removing and element doesn't
        // Change the rest of the items we need to process
        iter = 0;
        processed_updates++;
        const LightLevel light_level = lightMapGetType(
            current_chunk->lightmap,
            current_pos,
            LIGHT_TYPE_BLOCK
        );
        const VECTOR world_pos = vec3_add(
            current_pos,
            vec3_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        #pragma GCC unroll 6
        for (FaceDirection i = FACE_DIR_DOWN; i <= FACE_DIR_FRONT; i++) {
            const VECTOR query_pos = vec3_add(
                world_pos,
                WORLD_FACE_DIRECTION_NORMALS[i]
            );
            const IBlock* iblock = worldGetBlock(
                current_chunk->world,
                &query_pos
            );
            if (iblock == NULL) {
                continue;
            }
            const Block* block = VCAST_PTR(Block*, iblock);
            // Skip propogating light if we are facing a solid block
            // and the face in that direction is opaque
            if (!blockCanPropagateBlocklight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(i))) {
                continue;
            }
            const LightLevel neighbour_light_level = worldGetLightType(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_BLOCK
            );
            if (neighbour_light_level + 2 <= light_level) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    light_level - 1,
                    LIGHT_TYPE_BLOCK
                );
            }
        }
    }
    iter = 0;
    processed_updates = 0;
    while (lightCheckLimit(limits.add_sky, processed_updates)
            && hashmap_iter(chunk->updates.sunlight_add_queue, &iter, &item)) {
        const LightAddNode* node = item;
        const VECTOR current_pos = node->position;
        const Chunk* current_chunk = node->chunk;
        hashmap_delete(chunk->updates.sunlight_add_queue, node);
        iter = 0;
        processed_updates++;
        const LightLevel light_level = lightMapGetType(
            current_chunk->lightmap,
            current_pos,
            LIGHT_TYPE_SKY
        );
        const VECTOR world_pos = vec3_add(
            current_pos,
            vec3_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        #pragma GCC unroll 2
        for (FaceDirection i = FACE_DIR_DOWN; i <= FACE_DIR_UP; i++) {
            const VECTOR query_pos = vec3_add(
                world_pos,
                WORLD_FACE_DIRECTION_NORMALS[i]
            );
            const IBlock* iblock = worldGetBlock(
                current_chunk->world,
                &query_pos
            );
            if (iblock == NULL) {
                continue;
            }
            const Block* block = VCAST_PTR(Block*, iblock);
            // Skip propogating light if we are facing a solid block
            // and the face in that direction is opaque
            if (!blockCanPropagateSunlight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(i))) {
                continue;
            }
            const LightLevel neighbour_light_level = worldGetLightType(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_SKY
            );
            if (light_level == 15 && neighbour_light_level < 15) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    light_level,
                    LIGHT_TYPE_SKY
                );
            } else if (light_level < 15 && neighbour_light_level + 2 <= light_level) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    light_level - 1,
                    LIGHT_TYPE_SKY
                );
            }
        }
        #pragma GCC unroll 4
        for (FaceDirection i = FACE_DIR_LEFT; i <= FACE_DIR_FRONT; i++) {
            const VECTOR query_pos = vec3_add(
                world_pos,
                WORLD_FACE_DIRECTION_NORMALS[i]
            );
            const IBlock* iblock = worldGetBlock(
                current_chunk->world,
                &query_pos
            );
            if (iblock == NULL) {
                continue;
            }
            const Block* block = VCAST_PTR(Block*, iblock);
            // Skip propogating light if we are facing a solid block
            // and the face in that direction is opaque
            if (!blockCanPropagateSunlight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(i))) {
                continue;
            }
            const LightLevel neighbour_light_level = worldGetLightType(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_SKY
            );
            if (neighbour_light_level + 2 <= light_level) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    light_level - 1,
                    LIGHT_TYPE_SKY
                );
            }
        }
    }
}

void chunkUpdateRemoveLight(Chunk* chunk, const LightUpdateLimits limits) {
    chunk->lightmap_updated = hashmap_count(chunk->updates.light_remove_queue) != 0
                            || hashmap_count(chunk->updates.sunlight_remove_queue) != 0;
    size_t processed_updates = 0;
    size_t iter = 0;
    void* item;
    while (lightCheckLimit(limits.remove_block, processed_updates)
            && hashmap_iter(chunk->updates.light_remove_queue, &iter, &item)) {
        const LightRemoveNode* node = item;
        const VECTOR current_pos = node->position;
        const Chunk* current_chunk = node->chunk;
        const LightLevel light_level = node->light_value;
        hashmap_delete(chunk->updates.light_remove_queue, node);
        iter = 0;
        processed_updates++;
        const VECTOR world_pos = vec3_add(
            current_pos,
            vec3_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        #pragma GCC unroll 6
        for (FaceDirection face_dir = FACE_DIR_DOWN; face_dir <= FACE_DIR_FRONT; face_dir++) {
            const VECTOR query_pos = vec3_add(
                world_pos,
                WORLD_FACE_DIRECTION_NORMALS[face_dir]
            );
            const IBlock* iblock = worldGetBlock(
                current_chunk->world,
                &query_pos
            );
            if (iblock == NULL) {
                continue;
            }
            const Block* block = VCAST_PTR(Block*, iblock);
            // Skip propogating light if we are facing a solid block
            // and the face in that direction is opaque
            if (!blockCanPropagateBlocklight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(face_dir))) {
                continue;
            }
            const ChunkBlockPosition block_pos = worldToChunkBlockPosition(
                &query_pos,
                CHUNK_SIZE
            );
            const LightLevel neighbour_level = worldGetLightType(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_BLOCK
            );
            if (neighbour_level != 0 && neighbour_level < light_level) {
                worldRemoveLightTypeChunkBlock(
                    current_chunk->world,
                    &block_pos,
                    LIGHT_TYPE_BLOCK
                );
            } else if (neighbour_level >= light_level) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    neighbour_level,
                    LIGHT_TYPE_BLOCK
                );
            }
        }
    }
    processed_updates = 0;
    iter = 0;
    while (lightCheckLimit(limits.remove_sky, processed_updates)
           && hashmap_iter(chunk->updates.sunlight_remove_queue, &iter, &item)) {
        const LightRemoveNode* node = item;
        const VECTOR current_pos = node->position;
        const Chunk* current_chunk = node->chunk;
        const LightLevel light_level = node->light_value;
        hashmap_delete(chunk->updates.sunlight_remove_queue, node);
        iter = 0;
        processed_updates++;
        const VECTOR world_pos = vec3_add(
            current_pos,
            vec3_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        #pragma GCC unroll 2
        for (FaceDirection face_dir = FACE_DIR_DOWN; face_dir <= FACE_DIR_UP; face_dir++) {
            if (face_dir == FACE_DIR_UP && light_level == 15) {
                // Remove sunlight column below if the current level
                // is maxed, implying direct sunlight. Otherwise
                // perform the normal logic for removing light
                continue;
            }
            const VECTOR query_pos = vec3_add(
                world_pos,
                WORLD_FACE_DIRECTION_NORMALS[face_dir]
            );
            const IBlock* iblock = worldGetBlock(
                current_chunk->world,
                &query_pos
            );
            if (iblock == NULL) {
                continue;
            }
            const Block* block = VCAST_PTR(Block*, iblock);
            // Skip propogating light if we are facing a solid block
            // and the face in that direction is opaque
            if (!blockCanPropagateSunlight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(face_dir))) {
                continue;
            }
            const LightLevel neighbour_level = worldGetLightType(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_SKY
            );
            if ((light_level == 15 && neighbour_level == 15)
                || (neighbour_level != 0 && neighbour_level < light_level)) {
                worldRemoveLightType(
                    current_chunk->world,
                    &query_pos,
                    LIGHT_TYPE_SKY
                );
            } else if (light_level != 15 && neighbour_level >= light_level) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    neighbour_level,
                    LIGHT_TYPE_SKY
                );
            }
        }
        #pragma GCC unroll 4
        for (FaceDirection face_dir = FACE_DIR_LEFT; face_dir <= FACE_DIR_FRONT; face_dir++) {
            const VECTOR query_pos = vec3_add(
                world_pos,
                WORLD_FACE_DIRECTION_NORMALS[face_dir]
            );
            const IBlock* iblock = worldGetBlock(
                current_chunk->world,
                &query_pos
            );
            if (iblock == NULL) {
                continue;
            }
            const Block* block = VCAST_PTR(Block*, iblock);
            // Skip propogating light if we are facing a solid block
            // and the face in that direction is opaque
            if (!blockCanPropagateSunlight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(face_dir))) {
                continue;
            }
            const LightLevel neighbour_level = worldGetLightType(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_SKY
            );
            if (neighbour_level != 0 && neighbour_level < light_level) {
                worldRemoveLightType(
                    current_chunk->world,
                    &query_pos,
                    LIGHT_TYPE_SKY
                );
            } else if (neighbour_level >= light_level) {
                worldSetLightValue(
                    current_chunk->world,
                    &query_pos,
                    neighbour_level,
                    LIGHT_TYPE_SKY
                );
            }
        }
    }
}
