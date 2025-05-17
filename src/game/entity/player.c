#include "player.h"

#include "../../entity/entity.h"
#include "../../logging/logging.h"
#include "../../math/vector.h"
#include "../../ui/components/cursor.h"
#include "../../util/interface99_extensions.h"
#include "../../util/memory.h"
#include "../blocks/blocks.h"
#include "../items/items.h"
#include "../world/chunk/chunk_structure.h"
#include "../world/world_raycast.h"

// Forward declaration
FWD_DECL IBlock* worldModifyVoxelConstructed(const World* world,
                                             const VECTOR* position,
                                             BlockConstructor block_constructor,
                                             IItem* from_item,
                                             bool drop_item,
                                             IItem** item_result);
FWD_DECL bool worldModifyVoxel(const World* world,
                               const VECTOR* position,
                               IBlock* block,
                               bool drop_item,
                               IItem** item_result);
FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);

#define HEIGHT_INTERVALS 3
#define RADIUS_INTERVALS 2
const i32 player_collision_intervals_height[HEIGHT_INTERVALS] = {
    0,
    258048, // ONE_BLOCK * 0.9
    516096  // ONE_BLOCK * 1.8
};
const i32 player_collision_intervals_radius[RADIUS_INTERVALS] = { 0, 57344 };
const PhysicsObjectConfig player_physics_object_config = (PhysicsObjectConfig) {
    .jump_height = 120422, // ONE_BLOCK * 0.42 = 120422
    // ONE_BLOCK * 0.2 = 57344
    .radius = 57344, // Width: 0.6 => Radius: ONE_BLOCK * 0.3 = 86016
    .height = 516096, // ONE_BLOCK * 1.8 = 516096
    .step_height = 0, // TODO: Implement this
    .gravity = 22937, // ONE_BLOCK * 0.08 = 22937
    .collision_intervals = {
        .height_count = HEIGHT_INTERVALS,
        .radius_count = RADIUS_INTERVALS,
        .height = player_collision_intervals_height,
        .radius = player_collision_intervals_radius
    },
    .y_offset = 464486// ONE_BLOCK * 1.62
};
const PhysicsObjectUpdateHandlers player_physics_object_update_handlers = (PhysicsObjectUpdateHandlers) {
    .fall_handler = (PhysicsObjectFall) playerFallHandler
};

INLINE Player* playerNew() {
    Player* player = malloc(sizeof(Player));
    zeroed(player);
    return player;
}

void playerInit(Player* player) {
    entityInit(&player->entity);
    player->entity.health = PLAYER_MAX_HEALTH;
    player->entity.armour = 0;
    player->entity.air = PLAYER_MAX_AIR;
    DEBUG_LOG("Breaking state reset\n");
    breakingStateReset(player->breaking);
    Inventory* inventory = inventoryNew();
    Hotbar* hotbar = hotbarNew();
    hotbarInit(hotbar);
    inventoryInit(inventory, hotbar);
    DEBUG_LOG("IUI set\n");
    DYN_PTR(&player->hotbar, Hotbar, IUI, hotbar);
    DYN_PTR(&player->inventory, Inventory, IUI, inventory);
    DEBUG_LOG("Physics object init\n");
    iPhysicsObjectInit(
        &player->entity.physics_object,
        &player_physics_object_config,
        &player_physics_object_update_handlers
    );
}

void playerDestroy(const Player* player) {
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    free(inventory);
    free(hotbar);
}

void playerUpdateCamera(const Player* player) {
    Camera* camera = player->camera;
    const PhysicsObject* physics_object = &player->entity.physics_object;
    camera->rotation.vx = physics_object->rotation.pitch;
    camera->rotation.vy = physics_object->rotation.yaw;
    camera->position.vx = physics_object->position.vx;
    camera->position.vy = -physics_object->aabb.min.vy - PLAYER_CAMERA_OFFSET;
    camera->position.vz = physics_object->position.vz;
}

void playerUpdate(Player* player, World* world) {
    iPhysicsObjectUpdate(&player->entity.physics_object, world, player);
    playerUpdateCamera(player);
}

static bool player_damaged = false;

void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms) {
    const Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    hotbarRenderAttributes(
        player->entity.health,
        player_damaged,
        player->entity.armour,
        player->entity.air,
        player->entity.physics_object.flags.in_water,
        ctx,
        transforms
    );
    player_damaged = false;
    hotbarRenderSlots(hotbar, ctx, transforms);
    uiRender(&hotbar->ui, ctx, transforms);
    const Inventory* inventory = VCAST(Inventory*, player->inventory);
    InventorySlotGroups groups = inventory->ui.active
        ? INVENTORY_SLOT_GROUP_ALL
        : INVENTORY_SLOT_GROUP_NONE;
    if (inventory->ui.active) {
        uiCursorRender(
            &cursor,
            ctx,
            transforms
        );
    }
    inventoryRenderSlots(inventory, groups, ctx, transforms);
    uiRender(&inventory->ui, ctx, transforms);
}

void playerFallHandler(PhysicsObject* physics_object, const i32 distance, void* ctx) {
    // Player* player = (Player*) ctx;
    if (distance >= ONE_BLOCK * 3) {
        // NULL as the source indicates direct damage application
        // iEntityAttackFrom(&player->entity, NULL, (distance / ONE_BLOCK) - 3);
    }
}

void updateBreakingState(Player* player, const RayCastResult* result, const World* world) {
    BreakingState* state = &player->breaking;
    if ((state->block == NULL && result->block != NULL)
        || (state->block != NULL && result->block != NULL && !vec3_equal(state->position, result->pos))) {
        // Trigger remesh on old block chunk since the previous block
        // being targetted will still show being broken in world
        Chunk* chunk = worldGetChunk(world, &state->position);
        chunk->mesh_updated = true;
        *state = (BreakingState) {
            .position = result->pos,
            .block = result->block
        };
        const PhysicsObjectFlags* player_physics_flags = &player->entity.physics_object.flags;
        const Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
        breakingStateCalculateTicks(
            state,
            hotbarGetSelectSlot(hotbar).data.item,
            player_physics_flags->in_water,
            player_physics_flags->on_ground
        );
        breakingStateCalculateVisibility(state, world);
        return;
    }
    state->ticks_so_far += ONE;
    if (state->ticks_so_far < state->ticks_precise) {
        return;
    }
    // NOTE: By making sure we only get the item at the last tick when
    //       breaking means the OG bug of being able to mine with one
    //       tool then switch to another at the last second to preserve
    //       durability will still work.
    Hotbar* hotbar = VCAST_PTR(Hotbar*, &player->hotbar);
    Slot* slot = &hotbarGetSelectSlot(hotbar);
    IItem* iitem = slot->data.item;
    const Block* block = VCAST_PTR(Block*, state->block);
    const BlockID block_id = block->id;
    const ToolType block_tool_type = blockGetToolType(block_id);
    const ItemMaterial block_tool_material = blockGetToolMaterial(block_id);
    ToolType item_tool_type = TOOLTYPE_NONE;
    ItemMaterial item_tool_material = ITEMMATERIAL_NONE;
    if (iitem == NULL) {
        // No item in the player's hand
        goto remove_world_block;
    }
    Item* item = VCAST_PTR(Item*, iitem);
    const ItemID item_id = item->id;
    if (itemGetType(item_id) != ITEMTYPE_TOOL) {
        // Not a tool, so no durability decrement or tool breaking to handle
        goto remove_world_block;
    }
    item_tool_type = itemGetToolType(item_id);
    item_tool_material = itemGetMaterial(item_id);
    // NOTE: Using an unchecked decrement on the durability here
    //       allows us to keep another OG bug which is that getting
    //       a negative durability item wraps around and gives a
    //       high durability item.
    if (itemHasDurability(item_id) && --item->durability == 0) {
        VCALL(*iitem, destroy);
        itemDestroy(iitem);
        slot->data.item = NULL;
    }
remove_world_block:;
    const bool drop_item_on_break = blockCanHarvest(
        block_tool_type,
        block_tool_material,
        item_tool_type,
        item_tool_material,
        block
    );
    worldModifyVoxel(
        world,
        &result->pos,
        airBlockCreate(NULL, 0),
        drop_item_on_break,
        NULL
    );
    breakingStateReset(*state);
}

INLINE static bool playerInputHandlerAttack(const PlayerInputHandlerContext* ctx) {
    Player* player = ctx->player;
    // NOTE: This will probably hit framerate a decent bit
    //       while we are holding down the BINDING_ATTACK
    //       button. It's probably not going to be a problem
    //       but it might warrant trying to optimise the
    //       implementation to just use fixed point intead
    //       of doubles that are semi-hardware based with
    //       extra software support.
    const RayCastResult result = worldRayCastIntersection(
        ctx->world,
        player->camera,
        PLAYER_REACH_DISTANCE
    );
    // TODO: Add a state to the player for determining whether to
    //       instant break or not (like creative). If it's true
    //       we should invoke worldModifyVoxel here directly and
    //       ensure that no items are dropped.
    if (result.block != NULL) {
        const Block* block = VCAST_PTR(Block*, result.block);
        if (block->id != BLOCKID_AIR) {
            updateBreakingState(player, &result, ctx->world);
            return true;
        }
        return false;
    }
    Hotbar* hotbar = VCAST_PTR(Hotbar*, &player->hotbar);
    Slot* slot = &hotbarGetSelectSlot(hotbar);
    IItem* iitem = slot->data.item;
    if (iitem == NULL) {
        return false;
    }
    const ItemActionState action_state = VCALL(*iitem, attackAction);
    if (action_state != ITEM_ACTION_STATE_DESTROY) {
        return false;
    }
    VCALL(*iitem, destroy);
    itemDestroy(iitem);
    slot->data.item = NULL;
    return false;
}

INLINE static void playerInputHandlerUse(const PlayerInputHandlerContext* ctx) {
    Player* player = ctx->player;
    const PhysicsObject* physics_object = &player->entity.physics_object;
    const RayCastResult result = worldRayCastIntersection(
        ctx->world,
        player->camera,
        PLAYER_REACH_DISTANCE
    );
    // Interaction order:
    // 1. If the raycast hit a block and we are not sneaking
    //   a. Invoke the block use handler (returns bool)
    //     i. If true we consumed the input, exit
    //     ii. Otherwise continue
    // 2. If the current item is NULL
    //   a. Go to 4
    // 3. If the current item is a block
    //   a. Place the block in the direction of the normal
    //      returned by the raycast
    //   b. Decrement the stack size
    //   c. If the stack size is zero
    //     i. Remove the item
    //     ii. Exit
    //   d. Otherwise continue
    //   e. Else if the current item is a tool
    //     i. Invoke the item use handler (returns state)
    //     ii. If state equals DESTROY
    //       1. Destroy the item
    //       2. Exit
    //     iii. Else if state equals USED then exit
    //     iv. Otherwise continue
    // 4. If we are sneaking
    //   a. Invoke the block update handler (returns bool)
    const bool sneaking  = physics_object->flags.sneaking;
    Hotbar* hotbar = VCAST_PTR(Hotbar*, &player->hotbar);
    Slot* slot = &hotbarGetSelectSlot(hotbar);
    IItem* iitem = slot->data.item;
    if (result.block != NULL && !sneaking && VCALL(*result.block, useAction)) {
        return;
    }
    if (iitem == NULL) {
        goto block_update;
    }
    Item* item = VCAST_PTR(Item*, iitem);
    switch (itemGetType(item->id)) {
        case ITEMTYPE_BLOCK:
            if (result.block == NULL) {
                break;
            }
            if (item->id > BLOCK_COUNT) {
                errorAbort(
                    "[ERROR] Cannot create block from item \"%s\" (id: %d)\n",
                    itemGetName(item->id),
                    item->id
                );
                return;
            }
            const BlockConstructor block_constructor = block_constructors[item->id];
            if (block_constructor == NULL) {
                errorAbort(
                    "[ERROR] No constructor exists for block matching item \"%s\" (id: %d)\n",
                    itemGetName(item->id),
                    item->id
                );
                return;
            }
            IBlock* iblock = block_constructor(iitem, 0);
            const VECTOR place_position = vec3_add(result.pos, result.face);
            if (!VCALL(*iblock, canPlace, ctx->world, &place_position, &physics_object->aabb)) {
                item->stack_size++;
                VCALL(*iblock, destroy, false);
                return;
            }
            const bool modify_result = worldModifyVoxel(
                ctx->world,
                &place_position,
                iblock,
                false,
                NULL
            );
            if (!modify_result) {
                VCALL(*iblock, destroy, false);
                break;
            }
            Block* block = VCAST_PTR(Block*, iblock);
            // Compute the closest normal to the direction the camera
            // is facing, noting only the horizontal (x/z) axis since
            // no blocks can be placed upwards or downwards in orientation.
            // i.e. no buttons on the ceiling or anything like that.
            const VECTOR rotation = rotationToDirection5o(&player->camera->rotation);
            block->orientation = faceDirectionClosestNormal(vec3_i32(
                rotation.vx,
                0, /* No Y axis since block orientation is always horizontal */
                rotation.vz
            ));
            // Compute the opposing direction to the camera dominant normal
            // which gives us the orientation the block should face in the
            // world according to how the camera was orientated when placing.
            block->orientation = faceDirectionOpposing(block->orientation);
            /* NOTE: Checking equal to 0 and not less than or equal to 0
             *       allows for the bug that means a a stack size of 2^31 - 1
             *       which is effectively infinite
             */
            if (item->stack_size == 0) {
                VCALL(*iitem, destroy);
                itemDestroy(iitem);
                slot->data.item = NULL;
                return;
            }
            break;
        case ITEMTYPE_TOOL:;
            const ItemActionState action_state = VCALL(*iitem, useAction);
            switch (action_state) {
                case ITEM_ACTION_STATE_DESTROY:
                    VCALL(*iitem, destroy);
                    itemDestroy(iitem);
                    slot->data.item = NULL;
                case ITEM_ACTION_STATE_USED:
                    return;
                case ITEM_ACTION_STATE_NONE:
                    break;
            }
            break;
        case ITEMTYPE_RESOURCE:
        case ITEMTYPE_ARMOUR:
            break;
    }
block_update:
    if (sneaking) {
        // Event is consumed so we don't try to invoke the tool usage
        // on this block
        VCALL(*result.block, useAction);
    }
}

INLINE static void playerInputHandlerWorldInteraction(const Input* input, PlayerInputHandlerContext* ctx) {
    Player* player = ctx->player;
    const PADTYPE* pad = input->pad;
    bool breaking = false;
    if (isPressed(pad, BINDING_ATTACK)) {
        breaking = playerInputHandlerAttack(ctx);
    } else if (isPressed(pad, BINDING_USE)) {
        playerInputHandlerUse(ctx);
    }
    // If we are not holding down the BINDING_ATTACK
    // button, then we should discontinue breaking
    // and update the flag to stop the breaking overlay
    // and revert breaking progress
    if (player->breaking.block != NULL && !breaking) {
        player->breaking.reset_trigger = true;
    }
}

#define MOVE_AMOUNT ONE_BLOCK
// ONE_BLOCK * 0.3
#define SNEAK_MOVE_AMOUNT 86016

// This intentionally always returns false as it is the
// base level of input handling, everything else should
// take control on top of this (i.e. return true).
INLINE static InputHandlerState playerInputHandlerMovement(const Input* input, const PlayerInputHandlerContext* ctx) {
    const PADTYPE* pad = input->pad;
    Player* player = ctx->player;
    PhysicsObject* physics_object = &player->entity.physics_object;
    physics_object->move.forward = 0;
    physics_object->move.strafe = 0;
    physics_object->flags.jumping = false;
    physics_object->flags.sneaking = false;
    if (physics_object->flags.no_clip) {
        physics_object->velocity = vec3_i32(0, 0, 0);
    }
    if (input->pad->stat != 0) {
        // No input, don't bother updating from pads
        return INPUT_HANDLER_RELINQUISH_NO_DEBOUNCE;
    }
    if (isPressed(pad, BINDING_LOOK_UP)) {
        physics_object->rotation.pitch = positiveModulo(
            physics_object->rotation.pitch - (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    } else if (isPressed(pad, BINDING_LOOK_DOWN)) {
        physics_object->rotation.pitch = positiveModulo(
            physics_object->rotation.pitch + (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    }
    if (isPressed(pad, BINDING_LOOK_LEFT)) {
        physics_object->rotation.yaw = positiveModulo(
            physics_object->rotation.yaw + (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    } else if (isPressed(pad, BINDING_LOOK_RIGHT)) {
        physics_object->rotation.yaw = positiveModulo(
            physics_object->rotation.yaw - (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    }
    i32 move_amount = MOVE_AMOUNT;
    if (isPressed(pad, BINDING_JUMP)) {
        if (physics_object->flags.no_clip) {
            physics_object->velocity.vy = SNEAK_MOVE_AMOUNT;
        } else {
            physics_object->flags.jumping = true;
        }
    }
    if (isPressed(pad, BINDING_SNEAK)) {
        move_amount = SNEAK_MOVE_AMOUNT;
        if (physics_object->flags.no_clip) {
            physics_object->velocity.vy = -move_amount;
        } else {
            physics_object->flags.sneaking = true;
        }
    }
    if (isPressed(pad, BINDING_MOVE_FORWARD)) {
        physics_object->move.forward += move_amount;
    } else if (isPressed(pad, BINDING_MOVE_BACKWARD)) {
        physics_object->move.forward -= move_amount;
    }
    if (isPressed(pad, BINDING_MOVE_LEFT)) {
        physics_object->move.strafe -= move_amount;
    } else if (isPressed(pad, BINDING_MOVE_RIGHT)) {
        physics_object->move.strafe += move_amount;
    }
    return INPUT_HANDLER_RELINQUISH_NO_DEBOUNCE;
}

InputHandlerState playerInputHandler(const Input* input, void* ctx) {
    PlayerInputHandlerContext* context = ctx;
    if (input->pad->stat == 0) {
        playerInputHandlerWorldInteraction(input, context);
    }
    return playerInputHandlerMovement(input, context);
}

void playerInputHandlerDestroy(Input* input, void* ctx) {
    PlayerInputHandlerContext* context = ctx;
    free(context);
}

void playerRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("Player_registerInputHandler");
void Player_registerInputHandler(VSelf, Input* input, void* ctx) {
    VSELF(Player);
    PlayerInputHandlerContext* context = malloc(sizeof(PlayerInputHandlerContext));
    context->player = self;
    context->world = (World*) ctx;
    InputHandlerVTable handler = (InputHandlerVTable) {
        .ctx = context,
        .input_handler = playerInputHandler,
        .input_handler_destroy = playerInputHandlerDestroy
    };
    inputAddHandler(input, handler);
}

void playerDamage(VSelf, const i16 amount) ALIAS("Player_damage");
void Player_damage(VSelf, const i16 amount) {
    VSELF(Player);
    iEntityDamage(self, amount);
    if (amount > 0) {
        player_damaged = true;
    }
}
