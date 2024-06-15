#include "player.h"

#include "../util/interface99_extensions.h"
#include "../logging/logging.h"
#include "../math/vector.h"
#include "../game/world/world_raycast.h"
#include "../game/blocks/blocks.h"

// Forward declaration
FWD_DECL bool worldModifyVoxel(const World* world, const VECTOR* position, IBlock* block, IItem** item_result);

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

void playerUpdate(Player* player, World* world) {
    iPhysicsObjectUpdate(&player->physics_object, world, player);
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

void updateBreakingState(Player* player, const RayCastResult* result) {
    PlayerBreaking* state = &player->breaking;
    if ((uintptr_t) state->block != (uintptr_t) result->block) {
        state->progress = 0;
        state->position = result->pos;
        state->block = result->block;
        return;
    }
    // TODO: Update breaking progress based on block and tool
}

void playerInputHandlerWorldInteraction(const Input* input, const PlayerInputHandlerContext* ctx) {
    Player* player = ctx->player;
    const PADTYPE* pad = input->pad;
    bool breaking = false;
    if (isPressed(pad, BINDING_ATTACK)) {
        const RayCastResult result = worldRayCastIntersection(
            ctx->world,
            VCAST_PTR(Camera*, player->camera),
            PLAYER_REACH_DISTANCE
        );
        if (result.block != NULL) {
            updateBreakingState(player, &result);
            breaking = true;
            IItem* item = NULL;
            worldModifyVoxel(ctx->world, &result.pos, airBlockCreate(), &item);
        }
    } else if (isPressed(pad, BINDING_USE)) {
        const RayCastResult result = worldRayCastIntersection(
            ctx->world,
            VCAST_PTR(Camera*, player->camera),
            PLAYER_REACH_DISTANCE
        );
        // TODO: Use item or place itemblock if active hotbar slot has
        //       anything in it, handling decrementing stack sizes,
        //       durability, or updating the raycast intersected block
        //       if need be for interaction (i.e doors)
    }
    // If we are not holding down the BINDING_ATTACH
    // button, then we should discontinue breaking
    // and update the flag to stop the breaking overlay
    // and revert breaking progress
    if (!breaking) {
        player->breaking = (PlayerBreaking) {
            .progress = 0,
            .position = vec3_i32_all(0),
            .block = NULL
        };
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
        return false;
    }
    // Look controls
    if (isPressed(pad, BINDING_LOOK_UP)) {
        // Look up
        physics_object->rotation.pitch = positiveModulo(
            physics_object->rotation.pitch - (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    } else if (isPressed(pad, BINDING_LOOK_DOWN)) {
        // Look down
        physics_object->rotation.pitch = positiveModulo(
            physics_object->rotation.pitch + (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    }
    if (isPressed(pad, BINDING_LOOK_LEFT)) {
        // Look left
        physics_object->rotation.yaw = positiveModulo(
            physics_object->rotation.yaw + (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
    } else if (isPressed(pad, BINDING_LOOK_RIGHT)) {
        // Look right
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
    PlayerInputHandlerContext* context = ctx;
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
