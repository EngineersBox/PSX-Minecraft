#include "player.h"

#include <block.h>

#include "../util/interface99_extensions.h"

const PhysicsObjectConfig player_physics_object_config = (PhysicsObjectConfig) {
    .jump_height = 120422, // ONE_BLOCK * 0.42 = 120422
    .radius = 86016, // Width: 0.6 => Radius: ONE_BLOCK * 0.3 = 86016
    .height = 516096, // ONE_BLOCK * 1.8 = 516096
    .step_height = 0, // TODO
    .gravity = 0 // TODO
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
        .motion = (VECTOR) {0},
        .velocity = (VECTOR) {0},
        .move_forward = 0,
        .move_strafe = 0,
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
    player->physics_object.move_forward = 0;
    player->physics_object.move_strafe = 0;
    if (input->pad->stat != 0) {
        return false;
    }
    const PADTYPE* pad = input->pad;
    i32 move_amount = ONE_BLOCK;
    if (isPressed(pad, binding_jump)) {
        player->physics_object.flags.jumping = true;
        printf("[PLAYER] Set jump = true\n");
    }
    if (isPressed(pad, binding_sneak)) {
        move_amount = 86016; // ONE_BLOCK * 0.3 = 86016
        player->physics_object.flags.sneaking = true;
        printf("[PLAYER] Set sneaking = true\n");
    }
    if (isPressed(pad, binding_move_forward)) {
        player->physics_object.move_forward += move_amount;
        printf("[PLAYER] Move forward\n");
    } else if (isPressed(pad, binding_move_backward)) {
        player->physics_object.move_forward -= move_amount;
        printf("[PLAYER] Move backward]\n");
    }
    if (isPressed(pad, binding_move_left)) {
        player->physics_object.move_strafe -= move_amount;
        printf("[PLAYER] Move left\n");
    } else if (isPressed(pad, binding_move_right)) {
        player->physics_object.move_strafe += move_amount;
        printf("[PLAYER] Move right\n");
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
