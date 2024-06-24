#include "chunk.h"

#include <assets.h>
#include <asset_indices.h>
#include <cube.h>
#include <inline_c.h>
#include <stdbool.h>

#include "../../../logging/logging.h"
#include "../../../math/math_utils.h"
#include "../../../math/vector.h"
#include "../../../structure/cvector.h"
#include "../../../structure/cvector_utils.h"
#include "../../../structure/primitive/clip.h"
#include "../../../structure/primitive/direction.h"
#include "../../../structure/primitive/primitive.h"
#include "../../../util/interface99_extensions.h"
#include "../../items/items.h"
#include "../generation/noise.h"
#include "meshing/binary_greedy_mesher.h"

// Forward declaration
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR* position);

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

#define BREAKING_OVERLAY_SIZE ((BLOCK_SIZE >> 1) + 2)
// [0,1] -> [-SIZE,SIZE]
#define convertToVertex(v, shift, size) (size * (-1 + ((((v) & (1 << (shift))) >> (shift)) << 1)))

static void chunkRenderBreakingOverlay(Chunk* chunk,
                                       const BreakingState* breaking_state,
                                       RenderContext* ctx,
                                       Transforms* transforms) {
    // if (breaking_state->block == NULL) {
    //     // No target block, therefore nothing to break
    //     // and so no overlay to render
    //     return;
    // }
    if (chunk->position.vx != 0 || chunk->position.vz != 0) {
        return;
    }
    // NOTE: For rendering the breaking overlay we can do the following:
    //       1. Get the 1-3 visible faces' normals of the block
    //       2. Look up the blooks in adject to those faces
    //       3. If a given adject block is air, then render the breaking
    //          overlay texture on that face
    //       In order to render the overlay, we can just use the same method
    //       that we use for chunk rendering to transform each vertex of a block
    //       relative to the chunk into perspective space, in this case just for
    //       a given set of faces instead of the entire block though.
    const VECTOR position_offset = vec3_i32(
        (breaking_state->position.vx * BLOCK_SIZE) - 1,
        (-breaking_state->position.vy * BLOCK_SIZE) - 1,
        (breaking_state->position.vz * BLOCK_SIZE) - 1
    );
    int p;
    const TextureAttributes face_attribute = (TextureAttributes) {
        .u = (BLOCK_TEXTURE_SIZE * 2) + (breaking_state->ticks_so_far / breaking_state->ticks_per_stage) * BLOCK_TEXTURE_SIZE,
        .v = BLOCK_TEXTURE_SIZE * 14,
        .w = BLOCK_TEXTURE_SIZE,
        .h = BLOCK_TEXTURE_SIZE,
        .tint = {0, 0, 0, 0}
    };
    const RECT tex_window = (RECT){
        .w = 0,
        .h = 0,
        .x = 0,
        .y = 0
    };
    const Texture* texture = &textures[ASSET_TEXTURES_TERRAIN_INDEX];
    const u8 visible_sides_bitset = breaking_state->visible_sides_bitset;
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        if (((visible_sides_bitset >> i) & 0b1) == 0) {
            continue;
        }
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        #define createVert(_v) vec3_i16( \
            convertToVertex(CUBE_INDICES[i]._v, 0, BREAKING_OVERLAY_SIZE) + position_offset.vx + BREAKING_OVERLAY_SIZE, \
            convertToVertex(CUBE_INDICES[i]._v, 1, BREAKING_OVERLAY_SIZE) + position_offset.vy - BREAKING_OVERLAY_SIZE, \
            convertToVertex(CUBE_INDICES[i]._v, 2, BREAKING_OVERLAY_SIZE) + position_offset.vz + BREAKING_OVERLAY_SIZE \
        )
        const SVECTOR current_verts[4] = {
            createVert(v0),
            createVert(v1),
            createVert(v2),
            createVert(v3)
        };
        #undef createVert
        #undef convertToVertex
        gte_ldv3(
            &current_verts[0],
            &current_verts[1],
            &current_verts[2]
        );
        // Rotation, Translation and Perspective Triple
        gte_rtpt();
        gte_nclip();
        gte_stopz(&p);
        if (p < 0) {
            freePrimitive(ctx, sizeof(POLY_FT4));
            continue;
        }
        // Average screen Z result for four primtives
        gte_avsz3();
        gte_stotz(&p);
        if (p <= 0 || p >= ORDERING_TABLE_LENGTH) {
            freePrimitive(ctx, sizeof(POLY_FT4));
            continue;
        }
        // Initialize a textured quad primitive
        setPolyFT4(pol4);
        // Set the projected vertices to the primitive
        gte_stsxy0(&pol4->x0);
        gte_stsxy1(&pol4->x1);
        gte_stsxy2(&pol4->x2);
        // Compute the last vertex and set the result
        gte_ldv0(&current_verts[3]);
        gte_rtps();
        gte_stsxy(&pol4->x3);
        // Test if quad is off-screen, discard if so
        if (quadClip(
            &ctx->screen_clip,
            (DVECTOR*) &pol4->x0,
            (DVECTOR*) &pol4->x1,
            (DVECTOR*) &pol4->x2,
            (DVECTOR*) &pol4->x3)) {
            freePrimitive(ctx, sizeof(POLY_FT4));
            continue;
        }
        // Load primitive color even though gte_ncs() doesn't use it.
        // This is so the GTE will output a color result with the
        // correct primitive code.
        if (face_attribute.tint.cd) {
            setRGB0(
                pol4,
                face_attribute.tint.r,
                face_attribute.tint.g,
                face_attribute.tint.b
            );
        }
        gte_ldrgb(&pol4->r0);
        // Load the face normal
        gte_ldv0(&CUBE_NORMS[i]);
        // Apply RGB tinting to lighting calculation result on the basis
        // that it is enabled. This corresponds to the column based calc
        if (face_attribute.tint.cd) {
            // Normal Color Column Single
            gte_nccs();
        } else {
            // Normal Color Single
            gte_ncs();
        }
        // Store result to the primitive
        gte_strgb(&pol4->r0);
        // Set texture coords and dimensions
        setUVWH(
            pol4,
            face_attribute.u,
            face_attribute.v,
            face_attribute.w,
            face_attribute.h
        );
        // Bind texture page and colour look-up-table
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        u32* ot_entry = allocateOrderingTable(ctx, p);
        addPrim(ot_entry, pol4);
        // Bind texture window to ensure we don't accidently reference a window
        // from the chunk mesh rendering
        DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
        setTexWindow(ptwin, &tex_window);
        ot_entry = allocateOrderingTable(ctx, p);
        addPrim(ot_entry, ptwin);
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
                 const BreakingState* breaking_state,
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
    chunkMeshRender(&chunk->mesh, ctx, transforms);
    // TODO: Fix logic in world that ensures only one chunk gets non-NULL instance
    if (breaking_state != NULL) {
        chunkRenderBreakingOverlay(chunk, breaking_state, ctx, transforms);
    }
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

bool chunkModifyVoxel(Chunk* chunk,
                     const VECTOR* position,
                     IBlock* block,
                     const bool drop_item,
                     IItem** item_result) {
    const i32 x = position->vx;
    const i32 y = position->vy;
    const i32 z = position->vz;
    if (checkIndexOOB(x, y, z)) {
        return false;
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
    chunk->blocks[chunkBlockIndex(x, y, z)] = block;
    chunkClearMesh(chunk);
    chunkGenerateMesh(chunk);
    return result;
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
    while (true) {
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
        const int stack_left = itemGetMaxStackSize(slot_item->id) - slot_item->stack_size;
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
            printf("[ITEM] Picked up: %s x%d\n", itemGetName(item->id), item->stack_size);
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
