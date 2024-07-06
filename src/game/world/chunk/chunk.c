#include "chunk.h"

#include <cube.h>
#include <inline_c.h>
#include <stdbool.h>

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

// Forward declaration
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);
FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
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

void chunkInit(Chunk* chunk) {
    chunk->dropped_items = NULL;
    cvector_init(chunk->dropped_items, 0, chunkDestroyDroppedItem);
    chunk->updates = (ChunkUpdates) {
        .sunlight_queue = NULL,
        .light_add_queue = NULL,
        .light_remove_queue = NULL
    };
    cvector_init(chunk->updates.sunlight_queue, 0 ,NULL);
    cvector_init(chunk->updates.light_add_queue, 0, NULL);
    cvector_init(chunk->updates.light_remove_queue, 0, NULL);
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
}

void chunkDestroy(const Chunk* chunk) {
    chunkMeshDestroy(&chunk->mesh);
    cvector_free(chunk->dropped_items);
    cvector_free(chunk->updates.sunlight_queue);
    cvector_free(chunk->updates.light_add_queue);
    cvector_free(chunk->updates.light_remove_queue);
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
                printf("Height: %d\n", height);
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
}

void chunkGenerateMesh(Chunk* chunk) {
    chunkGenerateMeshWithBreakingState(chunk, NULL);
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
                      IBlock* block,
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
    )] = block;
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

void chunkUpdate(const Chunk* chunk, const Player* player) {
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
                        const u8 light_value,
                        const LightType light_type) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return;
    }
    lightMapSetValue(chunk->lightmap, *position, light_value, light_type);
}

void chunkUpdateAddLight(Chunk* chunk,
                         const VECTOR* position,
                         const u8 light_value,
                         const LightType light_type) {
    cvector_push_back(chunk->updates.light_add_queue, ((LightAddNode) {
        *position,
        chunk
    }));
    while (!cvector_empty(chunk->updates.light_add_queue)) {
        const VECTOR current_pos = chunk->updates.light_add_queue[0].position;
        Chunk* current_chunk = chunk->updates.light_add_queue[0].chunk;
        cvector_erase(chunk->updates.light_add_queue, 0);
        const u8 light_level = lightMapGetType(
            current_chunk->lightmap,
            current_pos,
            LIGHT_TYPE_BLOCK
        );
        VECTOR world_pos = vector_add(
            current_pos,
            vector_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        // TODO: For worldGetBlock it might be good to only return NULL
        //       when the position is outside of the loaded world.
        #pragma GCC unroll 6
        for (FaceDirection i = FACE_DIR_DOWN; i <= FACE_DIR_FRONT; i++) {
            const VECTOR query_pos = vector_add(
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
            if (blockGetType(block->id) == BLOCKTYPE_SOLID) {
                continue;
            }
            const u8 block_light_level = worldGetLightValue(
                current_chunk->world,
                &query_pos,
                LIGHT_TYPE_BLOCK
            );
            if (block_light_level + 2 > light_level) {
                continue;
            }
            worldSetLightValue(
                current_chunk->world,
                &query_pos,
                light_level - 1,
                LIGHT_TYPE_BLOCK
            );
            const ChunkBlockPosition block_pos = worldToChunkBlockPosition(
                &query_pos,
                CHUNK_SIZE
            );
            cvector_push_back(current_chunk->updates.light_add_queue, ((LightAddNode) {
                block_pos.block,
                worldGetChunk(
                    current_chunk->world,
                    &block_pos.chunk
                ),
            }));
        }
    }
}

void chunkUpdateRemoveLight(Chunk* chunk, const VECTOR* position) {
    u8 light_value = chunkGetLightValue(
        chunk,
        position,
        LIGHT_TYPE_BLOCK
    );
    cvector_push_back(chunk->updates.light_remove_queue, ((LightRemovalNode) {
        *position,
        chunk,
        light_value
    }));
    while (!cvector_empty(chunk->updates.light_remove_queue)) {
        const VECTOR current_pos = chunk->updates.light_remove_queue[0].position;
        Chunk* current_chunk = chunk->updates.light_remove_queue[0].chunk;
        u8 light_level = chunk->updates.light_remove_queue[0].light_value;
        cvector_erase(chunk->updates.light_remove_queue, 0);
        VECTOR world_pos = vector_add(
            current_pos,
            vector_const_mul(
                current_chunk->position,
                CHUNK_SIZE
            )
        );
        // TODO: For worldGetBlock it might be good to only return NULL
        //       when the position is outside of the loaded world.
        #pragma GCC unroll 6
        for (FaceDirection i = FACE_DIR_DOWN; i <= FACE_DIR_FRONT; i++) {
            const VECTOR query_pos = vector_add(
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
            if (blockGetType(block->id) == BLOCKTYPE_SOLID) {
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
                cvector_push_back(current_chunk->updates.light_remove_queue, ((LightRemovalNode) {
                    block_pos.block,
                    worldGetChunk(
                        current_chunk->world,
                        &block_pos.chunk
                    ),
                    neighbour_level
                }));
            } else if (neighbour_level >= light_level) {
                cvector_push_back(current_chunk->updates.light_add_queue, ((LightAddNode) {
                    block_pos.block,
                    worldGetChunk(
                        current_chunk->world,
                        &block_pos.chunk
                    ),
                }));
            }
        }
    }
}
