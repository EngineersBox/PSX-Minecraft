#include "player.h"

#include "../util/interface99_extensions.h"

void playerInit(Player* player) {
    Inventory* inventory = (Inventory*) malloc(sizeof(Inventory));
    Hotbar* hotbar = (Hotbar*) malloc(sizeof(Hotbar));
    hotbarInit(hotbar);
    inventoryInit(inventory, hotbar);
    DYN_PTR(&player->hotbar, Hotbar, IUI, hotbar);
    DYN_PTR(&player->inventory, Inventory, IUI, inventory);
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

void playerUpdate(Player* player) {
    const Camera* camera = VCAST_PTR(Camera*, player->camera);

}

bool playerInputHandler(const Input* input, void* ctx) {
    Player* player = (Player*) ctx;
    player->physics_object.move_forward = 0;
    player->physics_object.move_strafe = 0;
    if (input->pad->stat != 0) {
        return false;
    }
    const PADTYPE* pad = input->pad;
    i16 move_amount = ONE;
    if (isPressed(pad, binding_jump)) {
        player->physics_object.flags.jumping = true;
    }
    if (isPressed(pad, binding_sneak)) {
        move_amount = 1228;
        player->physics_object.flags.sneaking = true;
    }
    if (isPressed(pad, binding_move_forward)) {
        player->physics_object.move_forward += move_amount;
    } else if (isPressed(pad, binding_move_backward)) {
        player->physics_object.move_forward -= move_amount;
    }
    if (isPressed(pad, binding_move_left)) {
        player->physics_object.move_strafe -= move_amount;
    } else if (isPressed(pad, binding_move_right)) {
        player->physics_object.move_strafe += move_amount;
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
