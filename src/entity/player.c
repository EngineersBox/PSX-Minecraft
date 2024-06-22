#include "player.h"

#include <cvector_utils.h>
#include <items.h>

#include "../util/interface99_extensions.h"
#include "../logging/logging.h"
#include "../math/vector.h"
#include "../game/world/world_raycast.h"
#include "../game/blocks/blocks.h"

// Forward declaration
FWD_DECL bool worldModifyVoxel(const World* world,
                               const VECTOR* position,
                               IBlock* block,
                               bool drop_item,
                               IItem** item_result);

#define HEIGHT_INTERVALS 3
#define RADIUS_INTERVALS 2
const u32 player_collision_intervals_height[HEIGHT_INTERVALS] = { 0, 258048, 516096 };
const u32 player_collision_intervals_radius[RADIUS_INTERVALS] = { 0, 57344 };
const PhysicsObjectConfig player_physics_object_config = (PhysicsObjectConfig) {
    .jump_height = 120422, // ONE_BLOCK * 0.42 = 120422
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
    .y_offset = 6635 // ONE * 1.62
};
const PhysicsObjectUpdateHandlers player_physics_object_update_handlers = (PhysicsObjectUpdateHandlers) {
    .fall_handler = (PhysicsObjectFall) playerFallHandler
};

void playerInit(Player* player) {
    entityInit(&player->entity);
    player->entity.health = PLAYER_MAX_HEALTH;
    breakingStateReset(player->breaking);
    Inventory* inventory = (Inventory*) malloc(sizeof(Inventory));
    Hotbar* hotbar = (Hotbar*) malloc(sizeof(Hotbar));
    hotbarInit(hotbar);
    inventoryInit(inventory, hotbar);
    DYN_PTR(&player->hotbar, Hotbar, IUI, hotbar);
    DYN_PTR(&player->inventory, Inventory, IUI, inventory);
    iPhysicsObjectInit(
        &player->physics_object,
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
    Camera* camera = VCAST_PTR(Camera*, player->camera);
    const PhysicsObject* physics_object = &player->physics_object;
    camera->rotation.vx = physics_object->rotation.pitch;
    camera->rotation.vy = physics_object->rotation.yaw;
    camera->position.vx = physics_object->position.vx;
    camera->position.vy = -physics_object->position.vy - PLAYER_CAMERA_OFFSET;
    camera->position.vz = physics_object->position.vz;
}

void playerUpdate(Player* player, World* world) {
    iPhysicsObjectUpdate(&player->physics_object, world, player);
    playerUpdateCamera(player);
}

void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms) {
    const Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    hotbarRenderSlots(hotbar, ctx, transforms);
    uiRender(&hotbar->ui, ctx, transforms);
    const Inventory* inventory = VCAST(Inventory*, player->inventory);
    inventoryRenderSlots(inventory, ctx, transforms);
    uiRender(&inventory->ui, ctx, transforms);
}

void playerFallHandler(PhysicsObject* physics_object, const i32 distance, void* ctx) {
    Player* player = (Player*) ctx;
    if (distance >= ONE_BLOCK * 3) {
        // NULL as the source indicates direct damage application
        // iEntityAttackFrom(&player->entity, NULL, (distance / ONE_BLOCK) - 3);
    }
}

void updateBreakingState(Player* player, const RayCastResult* result, const World* world) {
    BreakingState* state = &player->breaking;
    if ((uintptr_t) state->block != (uintptr_t) result->block) {
        *state = (BreakingState) {
            .ticks_precise = 0,
            .ticks_per_stage = 0,
            .ticks_so_far = 0,
            .position = result->pos,
            .block = result->block
        };
        const PhysicsObjectFlags* player_physics_flags = &player->physics_object.flags;
        const Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
        DEBUG_LOG("Calculating breaking state ticks\n");
        DEBUG_LOG("Slot item: %p\n", hotbarGetSelectSlot(hotbar).data.item);
        Slot* slot;
        cvector_for_each_in(slot, hotbar->slots) {
            DEBUG_LOG("Slot %d: %p\n", slot->index, slot->data.item);
        }
        breakingStateCalculateTicks(
            state,
            hotbarGetSelectSlot(hotbar).data.item,
            player_physics_flags->in_water,
            player_physics_flags->on_ground
        );
        DEBUG_LOG("Breaking state: [ticks_precise: %d] [ticks_per_stage: %d]\n", state->ticks_precise, state->ticks_per_stage);
        return;
    }
    DEBUG_LOG("Equal blocks\n");
    state->ticks_so_far += ONE;
    if (--state->ticks_so_far < state->ticks_precise) {
        return;
    }
    breakingStateReset(*state);
    // NOTE: By making sure we only get the item at the last tick when
    //       breaking means the OG bug of being able to mine with one
    //       tool then switch to another at the last second to preserve
    //       durability will still work.
    const Hotbar* hotbar = VCAST_PTR(Hotbar*, &player->hotbar);
    Slot* slot = &hotbarGetSelectSlot(hotbar);
    IItem* iitem = slot->data.item;
    bool drop_item_on_break = false;
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
    const BlockID block_id = VCAST_PTR(Block*, state->block)->id;
    const ToolType block_tool_type = blockGetToolType(block_id);
    const ItemMaterial block_tool_material = blockGetToolMaterial(block_id);
    const ToolType item_tool_type = itemGetToolType(item_id);
    const ItemMaterial item_tool_material = itemGetMaterial(item_id);
    drop_item_on_break = item_tool_type == block_tool_type
                    && item_tool_material >= block_tool_material;
    // NOTE: Using an unchecked decrement on the durability here
    //       allows us to keep another OG bug which is that getting
    //       a negative durability item wraps around and gives a
    //       high durability item.
    if (itemHasDurability(item_id) && --item->durability == 0) {
        VCALL(*iitem, destroy);
        itemDestroy(iitem);
        slot->data.item = NULL;
    }
remove_world_block:
    worldModifyVoxel(
        world,
        &result->pos,
        airBlockCreate(),
        drop_item_on_break,
        NULL
    );
}

void playerInputHandlerWorldInteraction(const Input* input, const PlayerInputHandlerContext* ctx) {
    Player* player = ctx->player;
    const PADTYPE* pad = input->pad;
    bool breaking = false;
    if (isPressed(pad, BINDING_ATTACK)) {
        // NOTE: This will probably hit framerate a decent bit
        //       while we are holding down the BINDING_ATTACK
        //       button. It's probably not going to be a problem
        //       but it might warrant trying to optimise the
        //       implementation to just use fixed point intead
        //       of doubles that are semi-hardware based with
        //       extra software support.
        const RayCastResult result = worldRayCastIntersection(
            ctx->world,
            VCAST_PTR(Camera*, player->camera),
            PLAYER_REACH_DISTANCE
        );
        const Block* block = VCAST_PTR(Block*, result.block);
        // TODO: Add a state to the player for determining whether to
        //       instant break or not (like creative). If it's true
        //       we should invoke worldModifyVoxel here directly and
        //       ensure that no items are dropped.
        if (result.block != NULL && block->id != BLOCKID_AIR) {
            updateBreakingState(player, &result, ctx->world);
            breaking = true;
        }
    } else if (isPressed(pad, BINDING_USE)) {
        TODO(
            "Support USE actions for placing blocks and using items.\n"
            "Use item or place itemblock if active hotbar slot has\n"
            "anything in it, handling decrementing stack sizes,\n"
            "durability, or updating the raycast intersected block\n"
            "if need be for interaction (i.e doors)"
        );
        // const RayCastResult result = worldRayCastIntersection(
        //     ctx->world,
        //     VCAST_PTR(Camera*, player->camera),
        //     PLAYER_REACH_DISTANCE
        // );
    }
    // If we are not holding down the BINDING_ATTACK
    // button, then we should discontinue breaking
    // and update the flag to stop the breaking overlay
    // and revert breaking progress
    if (!breaking) {
        breakingStateReset(player->breaking);
    }
}

bool playerInputHandlerMovement(const Input* input, const PlayerInputHandlerContext* ctx) {
    const PADTYPE* pad = input->pad;
    Player* player = ctx->player;
    PhysicsObject* physics_object = &player->physics_object;
    physics_object->move.forward = 0;
    physics_object->move.strafe = 0;
    physics_object->flags.jumping = false;
    physics_object->flags.sneaking = false;
    if (physics_object->flags.no_clip) {
        physics_object->velocity = vec3_i32(0, 0, 0);
    }
    if (input->pad->stat != 0) {
        // No input, don't bother updating from pads
        return false;
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
    i32 move_amount = ONE_BLOCK;
    if (isPressed(pad, BINDING_JUMP)) {
        if (physics_object->flags.no_clip) {
            physics_object->velocity.vy = move_amount;
        } else {
            physics_object->flags.jumping = true;
        }
    }
    if (isPressed(pad, BINDING_SNEAK)) {
        move_amount = 86016; // ONE_BLOCK * 0.3 = 86016
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
    return false;
}

bool playerInputHandler(const Input* input, void* ctx) {
    const PlayerInputHandlerContext* context = ctx;
    playerInputHandlerWorldInteraction(input, context);
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
