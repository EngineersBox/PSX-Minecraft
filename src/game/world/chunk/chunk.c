#include "chunk.h"

#include <cube.h>
#include <inline_c.h>
#include <stdbool.h>
#include <string.h>

#include "../../../logging/logging.h"
#include "../../../math/math_utils.h"
#include "../../../math/vector.h"
#include "../../../structure/cvector.h"
#include "../../../structure/cvector_utils.h"
#include "../../../structure/primitive/direction.h"
#include "../../../util/interface99_extensions.h"
#include "../../items/items.h"
#include "../generation/noise.h"
#include "chunk_structure.h"
#include "meshing/binary_greedy_mesher.h"
#include "psxapi.h"

// Forward declaration
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);
FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
FWD_DECL Chunk* worldGetChunkFromChunkBlock(const World* world, const ChunkBlockPosition* position);
FWD_DECL u8 worldGetLightValue(const World* world,
                               const VECTOR* position,
                               const LightType light_type);
FWD_DECL void worldSetLightValue(const World* world,
                        const VECTOR* position,
                        u8 light_value,
                        const LightType light_type);
FWD_DECL void worldSetLightValueChunkBlock(const World* world,
                                           const ChunkBlockPosition* position,
                                           u8 light_value,
                                           const LightType light_type);

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
    memset(
        chunk->sunlight_heightmap,
        CHUNK_SIZE - 1,
        sizeof(*chunk->sunlight_heightmap) * CHUNK_SIZE * CHUNK_SIZE
    );
    chunk->dropped_items = NULL;
    cvector_init(chunk->dropped_items, 0, chunkDestroyDroppedItem);
    chunk->updates = (ChunkUpdates) {
        .sunlight_queue = NULL,
        .light_add_queue = NULL,
        .light_remove_queue = NULL
    };
    chunk->updates.sunlight_queue = hashmap_new(
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
        sizeof(LightAddNode),
        1,
        0,
        0,
        lightRemoveNodeDataHash,
        lightRemoveNodeCompare,
        NULL,
        NULL
    );
    /*cvector_init(chunk->updates.sunlight_queue, 0, NULL);*/
    /*cvector_init(chunk->updates.light_add_queue, 0, NULL);*/
    /*cvector_init(chunk->updates.light_remove_queue, 0, NULL);*/
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
    hashmap_free(chunk->updates.sunlight_queue);
    hashmap_free(chunk->updates.light_add_queue);
    hashmap_free(chunk->updates.light_remove_queue);
    /*cvector_free(chunk->updates.sunlight_queue);*/
    /*cvector_free(chunk->updates.light_add_queue);*/
    /*cvector_free(chunk->updates.light_remove_queue);*/
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

void chunkGenerateLightmap(Chunk* chunk) {
    bool sunlight_column_finished[CHUNK_SIZE * CHUNK_SIZE] = {false};
    u32 finished_count = 0;
    chunk->is_top = true;
    for (i32 y = CHUNK_SIZE - 1; y >= 0; y--) {
        for (i32 x = 0; x < CHUNK_SIZE; x++) {
            for (i32 z = 0; z < CHUNK_SIZE; z++) {
                if (sunlight_column_finished[(x * CHUNK_SIZE) + z]) {
                    continue;
                }
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
                    chunk->sunlight_heightmap[(x * CHUNK_SIZE) + z]--;
                } else {
                    sunlight_column_finished[(x * CHUNK_SIZE) + z] = true;
                    finished_count++;
                }
            }
        }
        if (finished_count >= CHUNK_SIZE * CHUNK_SIZE) {
            if (y == CHUNK_SIZE - 1) {
                // We have all solid blocks at the top layer
                // so we can just skip all sunlight generation.
                chunk->is_top = true;
            }
            break;
        }
    }
}

void chunkPropagateLightmap(Chunk* chunk) {
    if (!chunk->is_top) {
        return;
    }
    for (i32 y = CHUNK_SIZE - 1; y >= 0; y--) {
        for (i32 x = 0; x < CHUNK_SIZE; x++) {
            for (i32 z = 0; z < CHUNK_SIZE; z++) {
                if (chunk->sunlight_heightmap[(x *CHUNK_SIZE) + z] <= y) {
                    // We only want to determine if the sunlight
                    // columns have visible neighbours that are
                    // not in sunlight, so we skip any blocks that
                    // are not in the sunlight columns
                    continue;
                }
                const VECTOR position = vec3_i32(x, y, z);
                // Left, right, back, front
                for (FaceDirection face_dir = FACE_DIR_LEFT; face_dir <= FACE_DIR_FRONT; face_dir++) {
                    const VECTOR face_position = vec3_add(
                        vec3_i32(x, y, z),
                        FACE_DIRECTION_NORMALS[face_dir]
                    );
                    ChunkBlockPosition cb_pos = (ChunkBlockPosition) {
                        .chunk = chunk->position,
                        .block = face_position
                    };
                    const VECTOR query_pos = chunkBlockToWorldPosition(
                        &cb_pos,
                        CHUNK_SIZE
                    );
                    DEBUG_LOG(
                        "[CHUNK] Pos: " VEC_PATTERN " Face Pos: " VEC_PATTERN "\n",
                        VEC_LAYOUT(position),
                        VEC_LAYOUT(face_position)
                    );
                    cb_pos = worldToChunkBlockPosition(&query_pos, CHUNK_SIZE);
                    const Chunk* query_chunk = worldGetChunkFromChunkBlock(
                        chunk->world,
                        &cb_pos
                    );
                    if (query_chunk == NULL
                        || query_chunk->sunlight_heightmap[(cb_pos.block.vx * CHUNK_SIZE) + cb_pos.block.vz] < y) {
                        // Highest block in facing direction is lower than this
                        // block, so it's just sunlight. We don't need to propagate
                        // sunlight there since we already did that.
                        continue;
                    }
                    // Block in the facing direction is below the heightmap, so
                    // can potentially propagate skylight.
                    const IBlock* iblock = chunkGetBlockVec(chunk, &query_pos);
                    assert(iblock != NULL);
                    const Block* block = VCAST_PTR(Block*, iblock);
                    if (blockCanPropagateSunlight(block->id)
                        && !blockIsFaceOpaque(block, faceDirectionOpposing(face_dir))) {
                        // Light can propagate in this direction, so we have at
                        // least one valid direction. Add this block to the queue
                        // and skip the rest of the direction checks here.
                        DEBUG_LOG("[CHUNK] Can propagate to %s\n", blockGetName(block->id));
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

static void chunkRenderDroppedItems(const Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    IItem** item;
    cvector_for_each_in(item, chunk->dropped_items) {
        if (*item == NULL) {
            continue;
        }
        VCALL_SUPER(**item, Renderable, renderWorld, ctx, transforms);
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
                 BreakingState* breaking_state,
                 RenderContext* ctx,
                 Transforms* transforms) {
    if (breaking_state != NULL) {
        if (breaking_state->chunk_remesh_trigger) {
            // (In meshing) if the block matches the breaking state ensure it has unique mesh primitives
            // with correct tpage and texture window into the render target
            chunkClearMesh(chunk);
            chunkGenerateMeshWithBreakingState(chunk, breaking_state);
            breaking_state->chunk_remesh_trigger = false;
        }
        if (breaking_state->reset_trigger) {
            chunkClearMesh(chunk);
            chunkGenerateMesh(chunk);
            breakingStateReset(*breaking_state);
        }
    }
    if (chunk->lightmap_updated || chunk->mesh_updated) {
        // TODO: Do a mini version of the binary greedy meshing for
        //       lightmapping to generate a cached entry on each
        //       mesh primitive taht is just a 2D array of final light
        //       levels that can be applied when rendering the overalys
        //       as TILE_16 by looping over the texture coords (x,y)
        //       and indexing this cached map
        chunk->lightmap_updated = false;
        chunk->mesh_updated = false;
    }
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
    chunkMeshRender(&chunk->mesh, ctx, transforms);
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

#define HALF_BLOCK_SIZE (BLOCK_SIZE / 2)

static void constructItemPosition(const Chunk* chunk, const VECTOR* block_position, VECTOR* item_position) {
    // Construct vertices relative to chunk mesh bottom left origin
    const i16 chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    // Offset by 1 to ensure bottom block of bottom chunk starts at Y = 0
    const i16 chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
    const i16 chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
    // Positions only need to be somewhat accurate, so we will convert the position
    // to be in whole units, not fractional world space units. AKA just >> FIXED_POINT_SHIFT
    item_position->vx = (chunk_origin_x + block_position->vx) * BLOCK_SIZE + HALF_BLOCK_SIZE;
    item_position->vy = (chunk_origin_y - block_position->vy) * BLOCK_SIZE - HALF_BLOCK_SIZE;
    item_position->vz = (chunk_origin_z + block_position->vz) * BLOCK_SIZE + HALF_BLOCK_SIZE;
}

INLINE static int modifyVoxel0(Chunk* chunk,
                               const VECTOR* position,
                               const bool drop_item,
                               IItem** item_result) {
    const i32 x = position->vx;
    const i32 y = position->vy;
    const i32 z = position->vz;
    if (checkIndexOOB(x, y, z)) {
        return 2;
    }
    const IBlock* old_block = chunk->blocks[chunkBlockIndex(x, y, z)];
    const bool result = VCAST_PTR(Block*, old_block)->id != BLOCKID_AIR;
    IItem* iitem = VCALL(*old_block, destroy, drop_item);
    if (iitem != NULL && iitem->self != NULL) {
        cvector_push_back(
            chunk->dropped_items,
            iitem
        );
        Item* item = VCAST(Item*, *iitem);
        constructItemPosition(chunk, position, &item->position);
        if (item_result != NULL) {
            *item_result = iitem;
        }
    } else {
        if (item_result != NULL) {
            *item_result = NULL;
        }
    }
    return result;
}

bool chunkModifyVoxel(Chunk* chunk,
                      const VECTOR* position,
                      IBlock* iblock,
                      const bool drop_item,
                      IItem** item_result) {
    const int result = modifyVoxel0(chunk, position, drop_item, item_result);
    if (result == 2) {
        return false;
    }
    chunk->blocks[chunkBlockIndex(
        position->vx,
        position->vy,
        position->vz
    )] = iblock;
    /*const Block* block = VCAST_PTR(Block*, iblock);*/
    /*if (blockGetType(block->id) != BLOCKTYPE_EMPTY) {*/
    /*    chunkUpdateRemoveLight(chunk, position);*/
    /*} else {*/
    /*    // Adding a light level of 0 will trigger the lightmap*/
    /*    // to be regenerated around this point*/
    /*    chunkUpdateAddLight(*/
    /*        chunk,*/
    /*        position,*/
    /*        0,*/
    /*        LIGHT_TYPE_BLOCK*/
    /*    );*/
    /*}*/
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
    return true;
}

IBlock* chunkModifyVoxelConstructed(Chunk* chunk,
                                    const VECTOR* position,
                                    const BlockConstructor block_constructor,
                                    IItem* from_item,
                                    const bool drop_item,
                                    IItem** item_result) {
    const int result = modifyVoxel0(chunk, position, drop_item, item_result);
    if (result == 2) {
        return NULL;
    }
    IBlock* return_block = chunk->blocks[chunkBlockIndex(
        position->vx,
        position->vy,
        position->vz
    )] = block_constructor(from_item);
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
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

void chunkUpdate(Chunk* chunk, const Player* player) {
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
        if (itemUpdate(item, &pos, inventory, itemPickupValidator)) {
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
            printf("[ITEM] Picked up: %s x%d\n", itemGetName(item->id), item->stack_size);
            const InventoryStoreResult result = inventoryStoreItem(inventory, iitem);
            printf("[ITEM] Result: %s\n", inventoryStoreResultStringify(result));
            switch (result) {
                case INVENTORY_STORE_RESULT_ADDED_SOME:
                    // Do nothing, already updated iitem that was picked up as dropped.
                case INVENTORY_STORE_RESULT_NO_SPACE:
                    // Do nothing since we can't pick it up (don't think this will ever
                    // actually occur since we already check in itemPickupValidator for
                    // this case when determining which to items to consider.
                    i++;
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
                    break;
            }
            continue;
        }
        i++;
    }
    chunkUpdateLight(chunk);
}


u8 chunkGetLightValue(Chunk* chunk,
                      const VECTOR* position,
                      const LightType light_type) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return 0;
    }
    return lightMapGetType(chunk->lightmap, *position, light_type);
}

void chunkSetLightValue(Chunk* chunk,
                        const VECTOR* position,
                        u8 light_value,
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
            /*cvector_push_back(chunk->updates.light_add_queue, ((LightAddNode) {*/
            /*    *position,*/
            /*    chunk*/
            /*}));*/
            break;
        case LIGHT_TYPE_SKY:
            queue = chunk->updates.sunlight_queue;
            /*cvector_push_back(chunk->updates.sunlight_queue, ((LightAddNode) {*/
            /*    *position,*/
            /*    chunk*/
            /*}));*/
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
    const u8 light_value = lightMapGetType(
        chunk->lightmap,
        *position,
        light_type
    );
    const LightRemoveNode node = (LightRemoveNode) {
        *position,
        chunk,
        light_value
    };
    hashmap_set(chunk->updates.light_remove_queue, &node);
    if (hashmap_oom(chunk->updates.light_remove_queue)) {
        errorAbort("[CHUNK] Failed to enqueue light remove update, hashmap OOM");
    }
    /*cvector_push_back(chunk->updates.light_remove_queue, ((LightRemoveNode) {*/
    /*    *position,*/
    /*    chunk,*/
    /*    light_value*/
    /*}));*/
    lightMapSetValue(
        chunk->lightmap,
        *position,
        0,
        light_type
    );
}

void chunkUpdateLight(Chunk* chunk) {
    chunkUpdateRemoveLight(chunk);
    chunkUpdateAddLight(chunk);
}

#if CHUNK_LIGHT_ADD_BLOCK_UPDATES_PER_TICK >= 0
#define lightAddBlockCheckLimit(iter) ((iter) < CHUNK_LIGHT_ADD_BLOCK_UPDATES_PER_TICK)
#else
#define lightAddBlockCheckLimit(iter) true
#endif
#if CHUNK_LIGHT_ADD_SKY_UPDATES_PER_TICK >= 0
#define lightAddSkyCheckLimit(iter) ((iter) < CHUNK_LIGHT_ADD_SKY_UPDATES_PER_TICK)
#else
#define lightAddSkyCheckLimit(iter) true
#endif
#if CHUNK_LIGHT_REMOVE_UPDATES_PER_TICK >= 0
#define lightRemoveCheckLimit(iter) ((iter) < CHUNK_LIGHT_REMOVE_UPDATES_PER_TICK)
#else
#define lightRemoveCheckLimit(iter) true
#endif

void chunkUpdateAddLight(Chunk* chunk) {
    chunk->lightmap_updated = hashmap_count(chunk->updates.light_add_queue) != 0
                            || hashmap_count(chunk->updates.sunlight_queue) != 0;
    // Block light
    /*while (!cvector_empty(chunk->updates.light_add_queue)) {*/
    size_t processed_updates = 0;
    size_t iter = 0;
    void* item;
    /*DEBUG_LOG("[CHUNK] Add light queue: %d\n", hashmap_count(chunk->updates.light_add_queue));*/
    while (lightAddBlockCheckLimit(processed_updates)
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
        /*const VECTOR current_pos = chunk->updates.light_add_queue[0].position;*/
        /*Chunk* current_chunk = chunk->updates.light_add_queue[0].chunk;*/
        /*cvector_erase(chunk->updates.light_add_queue, 0);*/
        const u8 light_level = lightMapGetType(
            current_chunk->lightmap,
            current_pos,
            LIGHT_TYPE_BLOCK
        );
        VECTOR world_pos = vec3_add(
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
                FACE_DIRECTION_NORMALS[i]
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
            if (blockCanPropagateBlocklight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(i))) {
                continue;
            }
            const u8 neighbour_light_level = worldGetLightValue(
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
    /*DEBUG_LOG("[CHUNK] Add sunlight queue: %d\n", hashmap_count(chunk->updates.sunlight_queue));*/
    // Sky light
    /*while (!cvector_empty(chunk->updates.sunlight_queue)) {*/
    while (lightAddSkyCheckLimit(processed_updates)
            && hashmap_iter(chunk->updates.sunlight_queue, &iter, &item)) {
        const LightAddNode* node = item;
        const VECTOR current_pos = node->position;
        const Chunk* current_chunk = node->chunk;
        hashmap_delete(chunk->updates.sunlight_queue, node);
        iter = 0;
        processed_updates++;
        /*const VECTOR current_pos = chunk->updates.sunlight_queue[0].position;*/
        /*Chunk* current_chunk = chunk->updates.sunlight_queue[0].chunk;*/
        /*cvector_erase(chunk->updates.sunlight_queue, 0);*/
        const u8 light_level = lightMapGetType(
            current_chunk->lightmap,
            current_pos,
            LIGHT_TYPE_SKY
        );
        VECTOR world_pos = vec3_add(
            current_pos,
            vec3_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        // TODO: If the new light value is  15 (0b111) then we should propagate
        //       down without decrementing light value, then handle the L/R/F/B
        //       directions normally with decrementing light level
        #pragma GCC unroll 6
        for (FaceDirection i = FACE_DIR_DOWN; i <= FACE_DIR_FRONT; i++) {
            const VECTOR query_pos = vec3_add(
                world_pos,
                FACE_DIRECTION_NORMALS[i]
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
            if (blockCanPropagateSunlight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(i))) {
                continue;
            }
            const u8 neighbour_light_level = worldGetLightValue(
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

void chunkUpdateRemoveLight(Chunk* chunk) {
    chunk->lightmap_updated = hashmap_count(chunk->updates.light_remove_queue) != 0;
    /*while (!cvector_empty(chunk->updates.light_remove_queue)) {*/
    size_t processed_updates = 0;
    size_t iter;
    void* item;
    /*DEBUG_LOG("[CHUNK] Remove light queue: %d\n", hashmap_count(chunk->updates.light_remove_queue));*/
    while (lightRemoveCheckLimit(processed_updates)
            && hashmap_iter(chunk->updates.light_remove_queue, &iter, &item)) {
        const LightRemoveNode* node = item;
        const VECTOR current_pos = node->position;
        const Chunk* current_chunk = node->chunk;
        u8 light_level = node->light_value;
        hashmap_delete(chunk->updates.light_remove_queue, node);
        iter = 0;
        processed_updates++;
        /*const VECTOR current_pos = chunk->updates.light_remove_queue[0].position;*/
        /*Chunk* current_chunk = chunk->updates.light_remove_queue[0].chunk;*/
        /*u8 light_level = chunk->updates.light_remove_queue[0].light_value;*/
        /*cvector_erase(chunk->updates.light_remove_queue, 0);*/
        VECTOR world_pos = vec3_add(
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
                FACE_DIRECTION_NORMALS[i]
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
            if (blockCanPropagateBlocklight(block->id)
                || blockIsFaceOpaque(block, faceDirectionOpposing(i))) {
                continue;
            }
            const ChunkBlockPosition block_pos = worldToChunkBlockPosition(
                &query_pos,
                CHUNK_SIZE
            );
            const u8 neighbour_level = worldGetLightValue(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_BLOCK
            );
            if (neighbour_level != 0 && neighbour_level < light_level) {
                worldSetLightValueChunkBlock(
                    current_chunk->world,
                    &block_pos,
                    0,
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
}
