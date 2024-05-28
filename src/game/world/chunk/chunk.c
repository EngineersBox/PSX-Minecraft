#include "chunk.h"

#include <inline_c.h>
#include <stdbool.h>

#include "../../structure/primitive/clip.h"
#include "../../structure/cvector_utils.h"
#include "../../logging/logging.h"
#include "../../structure/cvector.h"
#include "../../structure/primitive/primitive.h"
#include "../generation/noise.h"
#include "../../math/math_utils.h"
#include "../../util/interface99_extensions.h"
#include "meshing/binary_greedy_mesher.h"

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
    // DEBUG_LOG("[CHUNK: %d,%d,%d] Initialising mesh\n", VEC_LAYOUT(chunk->position));
    chunkMeshInit(&chunk->mesh);
    chunkClearMesh(chunk);
    // DEBUG_LOG("[CHUNK: %d,%d,%d] Generating 2D height map\n", VEC_LAYOUT(chunk->position));
}

void chunkDestroy(const Chunk* chunk) {
    // DEBUG_LOG("[CHUNK: %d,%d,%d] Destroy mesh\n", VEC_LAYOUT(chunk->position));
    chunkMeshDestroy(&chunk->mesh);
    // DEBUG_LOG("[CHUNK: %d,%d,%d] Destroy dropped items: %d\n", VEC_LAYOUT(chunk->position), cvector_size(chunk->dropped_items));
    cvector_free(chunk->dropped_items);
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
                    ? airBlockCreate()
                    : stoneBlockCreate();
            }
        }
    }
}

void chunkClearMesh(Chunk* chunk) {
    chunkMeshClear(&chunk->mesh);
}

void chunkGenerateMesh(Chunk* chunk) {
    binaryGreedyMesherBuildMesh(chunk);
}

static void chunkRenderDroppedItems(Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    IItem** item;
    cvector_for_each_in(item, chunk->dropped_items) {
        if (*item == NULL) {
            continue;
        }
        VCALL_SUPER(**item, Renderable, renderWorld, ctx, transforms);
    }
}

bool chunkIsOutsideFrustum(const Chunk* chunk, const Frustum* frustum, const Transforms* transforms) {
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

void chunkRender(Chunk* chunk, RenderContext* ctx, Transforms* transforms) {
    if (chunkIsOutsideFrustum(chunk, &ctx->camera->frustum, transforms)) {
        return;
    }
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
        return airBlockCreate();
    }
    return chunk->blocks[chunkBlockIndex(x, y, z)];
}

IBlock* chunkGetBlockVec(const Chunk* chunk, const VECTOR* position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) {
        return airBlockCreate();
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

bool chunkModifyVoxel(Chunk* chunk, const VECTOR* position, IBlock* block, IItem** item_result) {
    const i32 x = position->vx;
    const i32 y = positiveModulo(-position->vy - 1, CHUNK_SIZE);
    const i32 z = position->vz;
    if (checkIndexOOB(x, y, z)) {
        return false;
    }
    const IBlock* old_block = chunk->blocks[chunkBlockIndex(x, y, z)];
    IItem* iitem = VCALL(*old_block, destroy);
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
    } else if (item_result != NULL) {
        *item_result = NULL;
    }
    chunk->blocks[chunkBlockIndex(x, y, z)] = block;
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
    return true;
}

// This only works because PS1 games are single threaded (mostly)
Inventory* _current_inventory = NULL;

bool itemPickupValidator(const Item* item) {
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
    while (1) {
        const Slot* slot = inventorySearchItem(_current_inventory, item->id, from_slot, &next_free);
        if (slot == NULL) {
            if (next_free == INVENTORY_NO_FREE_SLOT) {
                return false;
            }
            slot = &_current_inventory->slots[next_free];
            if (inventorySlotGetItem(slot) == NULL) {
                break;
            }
        }
        const Item* slot_item = VCAST(Item*, *inventorySlotGetItem(slot));
        const int stack_left = slot_item->max_stack_size - slot_item->stack_size;
        if (stack_left != 0) {
            break;
        }
        from_slot = slot->index + 1;
        next_free = INVENTORY_NO_FREE_SLOT;
    }
    return true;
}

void chunkUpdate(Chunk* chunk, Player* player) {
    _current_inventory = VCAST(Inventory*, player->inventory);
    // We are using chunk relative coords in absolute units and not in
    // fixed point format since item positons only need to be relatively
    // accurate not exact so we can save on the extra caclulation overhead
    // of needing to use division for worldspace accuracy.
    // @see itemPickupValidator
    const VECTOR pos = vec3_i32(
        player->physics_object.position.vx >> FIXED_POINT_SHIFT,
        player->physics_object.position.vy >> FIXED_POINT_SHIFT,
        player->physics_object.position.vz >> FIXED_POINT_SHIFT
    );
    for (u32 i = 0; i < cvector_size(chunk->dropped_items);) {
        IItem* iitem = chunk->dropped_items[i];
        if (iitem == NULL) {
            i++;
            continue;
        }
        Item* item = VCAST(Item*, *iitem);
        if (itemUpdate(item, &pos, itemPickupValidator)) {
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
            printf("[ITEM] Picked up: %s x%d\n", item->name, item->stack_size);
            const InventoryStoreResult result = inventoryStoreItem(_current_inventory, iitem);
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
