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
    uiRender(VCAST(const UI*, player->hotbar), ctx, transforms);
    uiRender(VCAST(const UI*, player->inventory), ctx, transforms);
}