#include "player.h"

#include "../game/blocks/block.h"
#include "../util/interface99_extensions.h"
#include "../debug/debug.h"

#define HEIGHT_INTERVALS 3
#define RADIUS_INTERVALS 2
const u32 player_collision_intervals_height[HEIGHT_INTERVALS] = { 0, 258048, 516096 };
const u32 player_collision_intervals_radius[RADIUS_INTERVALS] = { 0, 86016 };
const PhysicsObjectConfig player_physics_object_config = (PhysicsObjectConfig) {
    .jump_height = 120422, // ONE_BLOCK * 0.42 = 120422
    .radius = 86016, // Width: 0.6 => Radius: ONE_BLOCK * 0.3 = 86016
    .height = 516096, // ONE_BLOCK * 1.8 = 516096
    .step_height = 0, // TODO: Implement this
    .gravity = 22937, // ONE_BLOCK * 0.08 = 22937
    .collision_intervals = {
        .height_count = HEIGHT_INTERVALS,
        .radius_count = RADIUS_INTERVALS,
        .height = player_collision_intervals_height,
        .radius = player_collision_intervals_radius
    }
};

void playerInit(Player* player) {
    Inventory* inventory = (Inventory*) malloc(sizeof(Inventory));
    Hotbar* hotbar = (Hotbar*) malloc(sizeof(Hotbar));
    hotbarInit(hotbar);
    inventoryInit(inventory, hotbar);
    DYN_PTR(&player->hotbar, Hotbar, IUI, hotbar);
    DYN_PTR(&player->inventory, Inventory, IUI, inventory);
    player->physics_object = (PhysicsObject) {
        .position = (VECTOR) {0},
        .rotation = {
            .pitch = 0,
            .yaw = 0
        },
        .velocity = (VECTOR) {0},
        .move = {
            .forward = 0,
            .strafe = 0
        },
        .flags = {0},
        .config = &player_physics_object_config
    };
}

void playerDestroy(const Player* player) {
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    free(inventory);
    free(hotbar);
}

void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms) {
    const Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    hotbarRenderSlots(hotbar, ctx, transforms);
    uiRender(&hotbar->ui, ctx, transforms);
    const Inventory* inventory = VCAST(Inventory*, player->inventory);
    inventoryRenderSlots(inventory, ctx, transforms);
    uiRender(&inventory->ui, ctx, transforms);
}

bool playerInputHandler(const Input* input, void* ctx) {
    Player* player = (Player*) ctx;
    PhysicsObject* physics_object = &player->physics_object;
    physics_object->move.forward = 0;
    physics_object->move.strafe = 0;
    if (input->pad->stat != 0) {
        return false;
    }
    const PADTYPE* pad = input->pad;
    // Look controls
    if (isPressed(pad, binding_look_up)) {
        // Look up
        physics_object->rotation.pitch = positiveModulo(
            physics_object->rotation.pitch - (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
        // DEBUG_LOG("[PLAYER] Look up: %d\n", physics_object->rotation.pitch);
    } else if (isPressed(pad, binding_look_down)) {
        // Look down
        physics_object->rotation.pitch = positiveModulo(
            physics_object->rotation.pitch + (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
        // DEBUG_LOG("[PLAYER] Look dowm: %d\n", physics_object->rotation.pitch);
    }
    if (isPressed(pad, binding_look_left)) {
        // Look left
        physics_object->rotation.yaw = positiveModulo(
            physics_object->rotation.yaw + (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
        // DEBUG_LOG("[PLAYER] Look left: %d\n", physics_object->rotation.yaw);
    } else if (isPressed(pad, binding_look_right)) {
        // Look right
        physics_object->rotation.yaw = positiveModulo(
            physics_object->rotation.yaw - (ONE * ROTATION_SPEED),
            ONE << FIXED_POINT_SHIFT
        );
        // DEBUG_LOG("[PLAYER] Look right: %d\n", physics_object->rotation.yaw);
    }
    i32 move_amount = ONE_BLOCK;
    if (isPressed(pad, binding_jump)) {
        physics_object->flags.jumping = true;
        // DEBUG_LOG("[PLAYER] Set jump = true\n");
    }
    if (isPressed(pad, binding_sneak)) {
        move_amount = 86016; // ONE_BLOCK * 0.3 = 86016
        physics_object->flags.sneaking = true;
        // DEBUG_LOG("[PLAYER] Set sneaking = true\n");
    }
    if (isPressed(pad, binding_move_forward)) {
        physics_object->move.forward += move_amount;
    } else if (isPressed(pad, binding_move_backward)) {
        physics_object->move.forward -= move_amount;
    }
    if (isPressed(pad, binding_move_left)) {
        physics_object->move.strafe -= move_amount;
    } else if (isPressed(pad, binding_move_right)) {
        physics_object->move.strafe += move_amount;
    }
    return false;
}

void playerRegisterInputHandler(VSelf, Input* input) __attribute__((alias("Player_registerInputHandler")));
void Player_registerInputHandler(VSelf, Input* input) {
    VSELF(Player);
    ContextualInputHandler handler = (ContextualInputHandler) {
        .ctx = self,
        .input_handler = playerInputHandler
    };
    inputAddHandler(input, handler);
}
