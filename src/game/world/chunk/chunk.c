#include "chunk.h"

#include <cube.h>
#include <inline_c.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <psxapi.h>
#include <assert.h>

#include "../../../logging/logging.h"
#include "../../../math/math_utils.h"
#include "../../../math/vector.h"
#include "../../../render/duration_tree.h"
#include "../../../structure/cvector.h"
#include "../../../structure/cvector_utils.h"
#include "../../../structure/primitive/direction.h"
#include "../../../util/interface99_extensions.h"
#include "../../items/items.h"
#include "../generation/noise.h"
#include "chunk_mesh.h"
#include "chunk_structure.h"
#include "heightmap.h"
#include "meshing/binary_greedy_mesher.h"

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
FWD_DECL ChunkHeightmap* worldGetChunkHeightmap(World* world, const VECTOR* position);

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
                    ? airBlockCreate(NULL,0 )
                    : stoneBlockCreate(NULL, 0);
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
    DroppedIItem* dropped;
    cvector_for_each_in(dropped, chunk->dropped_items) {
        VCALL_SUPER(*dropped->iitem, Renderable, renderWorld, chunk, ctx, transforms);
    }
}

#if isOverlayEnabled(DURATION_TREE)
static char chunk_render_duration_names[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT][30] = {0};
static DurationComponent* chunk_render_duration[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT] = {0};
#endif

#define shiftChunkPos(chunk, axis) positiveModulo((chunk->position.axis + (AXIS_CHUNKS >> 1)), AXIS_CHUNKS)

void chunkRender(Chunk* chunk,
                 bool subdivide,
                 RenderContext* ctx,
                 Transforms* transforms) {
#if isOverlayEnabled(DURATION_TREE)
    DurationComponent** duration = &chunk_render_duration[shiftChunkPos(chunk, vx)]
                                                         [shiftChunkPos(chunk, vz)]
                                                         [chunk->position.vy];
    char* name = chunk_render_duration_names[shiftChunkPos(chunk, vx)]
                                            [shiftChunkPos(chunk, vz)]
                                            [chunk->position.vy % WORLD_CHUNKS_HEIGHT];
    if (*duration == NULL) {
        *duration = durationTreeAddComponent(name);
        assert(*duration != NULL);
    }
    sprintf(
        name,
        VEC_PATTERN,
        shiftChunkPos(chunk, vx),
        shiftChunkPos(chunk, vz),
        chunk->position.vy % WORLD_CHUNKS_HEIGHT
    );
    durationComponentStart(*duration);
#endif
#undef shiftChunkPos
    const AABB aabb = (AABB) {
        .min = vec3_i32(
            (chunk->position.vx * CHUNK_BLOCK_SIZE),// << FIXED_POINT_SHIFT,
            (-chunk->position.vy * CHUNK_BLOCK_SIZE),// << FIXED_POINT_SHIFT,
            (chunk->position.vz * CHUNK_BLOCK_SIZE)// << FIXED_POINT_SHIFT
        ),
        .max = vec3_i32(
            ((chunk->position.vx + 1) * CHUNK_BLOCK_SIZE),// << FIXED_POINT_SHIFT,
            (-(chunk->position.vy + 1) * CHUNK_BLOCK_SIZE),// << FIXED_POINT_SHIFT,
            ((chunk->position.vz + 1) * CHUNK_BLOCK_SIZE)// << FIXED_POINT_SHIFT
        )
    };
    /*if (frustumContainsAABB(&ctx->camera->frustum, &aabb) == FRUSTUM_OUTSIDE) {*/
        /*DEBUG_LOG("[CHUNK " VEC_PATTERN "] Not visible\n", VEC_LAYOUT(chunk->position));*/
    /*    durationComponentEnd();*/
    /*    return;*/
    /*} else {*/
        /*DEBUG_LOG("[CHUNK " VEC_PATTERN "] Visible\n", VEC_LAYOUT(chunk->position));*/
    /*}*/
    renderCtxBindMatrix(
        ctx,
        transforms,
        &VEC3_I16_ZERO,
        &chunk->position
    );
    // Sort + render mesh
    chunkMeshRender(
        &chunk->mesh,
        worldGetInternalLightLevel(chunk->world),
        &aabb,
        subdivide,
        ctx,
        transforms
    );
    renderCtxUnbindMatrix();
    chunkRenderDroppedItems(chunk, ctx, transforms);
    durationComponentEnd();
}

IBlock* chunkGetBlock(const Chunk* chunk, const i32 x, const i32 y, const i32 z) {
    assert(chunkBlockIndexInBounds(x, y, z));
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position) {
    assert(chunkBlockIndexInBounds(position->vx, position->vy, position->vz));
    return chunk->blocks[chunkBlockIndex(
        position->vx,
        position->vy,
        position->vz
    )];
}

#define HALF_BLOCK_SIZE (BLOCK_SIZE >> 1)

#define randMax(_max) (positiveModulo(rand(), ((_max) << 1)) - (_max)) 
#define randBounded(bound) ((rand() % 2 ? 1 : -1) * (positiveModulo(rand(), (bound)) + ONE))

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
    iPhysicsObjectSetPosition(
        &item->world_entity->physics_object,
        &item_position
    );
    item->world_entity->physics_object.rotation.yaw = rand() % 32768;
    item->world_entity->physics_object.rotation.pitch  = rand() % 32768;
    item->world_entity->physics_object.move.forward = randBounded(FIXED_1_2);
    item->world_entity->physics_object.move.strafe = randBounded(FIXED_1_2);
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

static u32 findClosestBlockBelow(Chunk* chunk,
                                 const u32 x,
                                 u32 y,
                                 const u32 z) {
    VECTOR chunk_position = chunk->position;
    while (true) {
        if (y == 0) {
            if (chunk_position.vy == 0) {
                return 0;
            }
            chunk_position.vy--;
            chunk = worldGetChunk(chunk->world, &chunk_position);
            y = CHUNK_SIZE;
        }
        y--;
        const IBlock* iblock = chunk->blocks[chunkBlockIndex(x, y, z)];
        const Block* block = VCAST_PTR(Block*, iblock);
        if (block->id != BLOCKID_AIR) {
            return chunk_position.vy + y;
        }
    }
}

static int modifyVoxel0(Chunk* chunk,
                        const VECTOR* position,
                        const Block* new_block,
                        const bool drop_item,
                        IItem** item_result) {
    const i32 x = position->vx;
    const i32 y = position->vy;
    const i32 z = position->vz;
    if (chunkBlockIndexOOB(x, y, z)) {
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
            ((DroppedIItem) {
                .iitem = iitem,
                .lifetime = time_ms + ITEM_DROPPED_LIFETIME_MS
            })
        );
        Item* item = VCAST(Item*, *iitem);
        applyItemWorldState(chunk, item, position);
        if (item_result != NULL) {
            *item_result = iitem;
        }
    } else if (item_result != NULL) {
        *item_result = NULL;
    }
    ChunkHeightmap* heightmap = worldGetChunkHeightmap(chunk->world, &chunk->position);
    const u32 index = chunkHeightmapIndex(x, z);
    const u32 top = (*heightmap)[index];
    const u32 world_y = chunk->position.vy + y;
    const bool new_is_air = new_block->id == BLOCKID_AIR;
    if (top == world_y && new_is_air) {
        // NOTE: Depending on how many chunks are loaded vertically,
        //       this could be quite expensive, meaning it might be
        //       better to have a queue for heightmap updates that
        //       we only do a certain amount of each tick in the same
        //       manner as lighting to armortise the performance
        //       impact.
        (*heightmap)[index] = findClosestBlockBelow(chunk, x, y, z);
    } else if (top < world_y && !new_is_air) {
        (*heightmap)[index] = world_y;
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
    // Using metadata ID of 0 with a valid template item (i.e from_item)
    // implies construction using the full item context and subsequently
    // the metadata id of the item as well
    IBlock* iblock = block_constructor(from_item, 0);
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
    Inventory* inventory = (Inventory*) ctx;
    // 1. Does the item already exist in the inventory?
    //   a. [1:TRUE] Does the existing have space?
    //     i. [a:TRUE] Return true
    //     ii. [a:FALSE] Go to 2
    //   b. [1:FALSE] Go to 2
    // 2. Is there space in the inventory
    //   a. [2:TRUE] Return true
    //   b. [2:FALSE] Return false
    u8 from_slot = slotGroupIndexOffset(INVENTORY_MAIN);
    u8 next_free = INVENTORY_NO_FREE_SLOT;
    while (true) {
        const Slot* slot = inventorySearchItem(
            inventory,
            item->id,
            item->metadata_id,
            from_slot,
            &next_free
        );
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
                              DroppedIItem* dropped,
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
    const Item* item = VCAST_PTR(Item*, dropped->iitem);
    if (!vec3_equal(item->world_entity->physics_object.velocity, VEC3_I32_ZERO)) {
        // No velocity implies no movement, so we can't have changed
        // chunks since we last checked
        return;
    }
    const VECTOR item_world_pos = vec3_const_div(
        item->world_entity->physics_object.position,
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
    cvector_push_back(new_chunk->dropped_items, *dropped);
}

void chunkUpdate(Chunk* chunk, const Player* player, BreakingState* breaking_state) {
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    // We are using chunk relative coords in absolute units and not in
    // fixed point format since item positons only need to be relatively
    // accurate not exact so we can save on the extra caclulation overhead
    // of needing to use division for worldspace accuracy.
    // @see itemPickupValidator
    const VECTOR pos = vec3_i32(
        player->entity.physics_object.position.vx >> FIXED_POINT_SHIFT,
        -player->entity.physics_object.position.vy >> FIXED_POINT_SHIFT,
        player->entity.physics_object.position.vz >> FIXED_POINT_SHIFT
    );
    // NOTE: I've made the cvector size and capacity fields
    //       volatile so that any usage in a loop that mutates
    //       the cvector will not run into issues with compiler
    //       optimisations around reads in loop conditions.
    for (u32 i = 0; i < cvector_size(chunk->dropped_items);) {
        DroppedIItem* dropped = &chunk->dropped_items[i];
        if (dropped->iitem == NULL) {
            i++;
            continue;
        } else if (time_ms - dropped->lifetime >= ITEM_DROPPED_LIFETIME_MS) {
            VCALL(*dropped->iitem, destroy);
            itemDestroy(dropped->iitem);
            cvector_erase(chunk->dropped_items, i);
            continue;
        }
        Item* item = VCAST(Item*, *dropped->iitem);
        if (itemUpdate(item,
                       chunk->world,
                       &pos,
                       inventory,
                       itemPickupValidator)) {
            const InventoryStoreResult result = inventoryStoreItem(inventory, dropped->iitem);
            switch (result) {
                case INVENTORY_STORE_RESULT_ADDED_SOME:
                    // Do nothing, already updated iitem that was picked up as dropped.
                    FALLTHROUGH;
                case INVENTORY_STORE_RESULT_NO_SPACE:
                    // Do nothing since we can't pick it up (don't think this will ever
                    // actually occur since we already check in itemPickupValidator for
                    // this case when determining which to items to consider.
                    break;
                case INVENTORY_STORE_RESULT_ADDED_ALL:
                    // Nuke it, added all so this item instance is not needed any more.
                    // Break is not used here since we still need to erase this array
                    // entry.
                    VCALL(*dropped->iitem, destroy);
                    itemDestroy(dropped->iitem);
                    FALLTHROUGH;
                case INVENTORY_STORE_RESULT_ADDED_NEW_SLOT:
                    // We reuse this item instance as the inventory instance now so don't
                    // free it, just remove the array entry (no element destructor set
                    // on the cvector instance)
                    cvector_erase(chunk->dropped_items, i);
                    continue;
                    break;
            }
        }
        updateItemChunkOwnership(chunk, dropped, i);
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
    assert(chunkBlockIndexInBounds(position->vx, position->vy, position->vz));
    return lightMapGetValue(chunk->lightmap, *position);
}

LightLevel chunkGetLightType(const Chunk* chunk,
                             const VECTOR* position,
                             const LightType light_type) {
    assert(chunkBlockIndexInBounds(position->vx, position->vy, position->vz));
    return lightMapGetType(chunk->lightmap, *position, light_type);
}

void chunkSetLightValue(Chunk* chunk,
                        const VECTOR* position,
                        const LightLevel light_value,
                        const LightType light_type) {
    assert(chunkBlockIndexInBounds(position->vx, position->vy, position->vz));
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
    assert(chunkBlockIndexInBounds(position->vx, position->vy, position->vz));
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
