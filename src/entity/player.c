#include "player.h"

#include "../util/interface99_extensions.h"

void playerInit(Player* player) {
    Inventory* inventory = (Inventory*) malloc(sizeof(Inventory));
    Hotbar* hotbar = (Hotbar*) malloc(sizeof(Hotbar));
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
    uiRender(VCAST(UI*, player->hotbar), ctx, transforms);
    uiRender(VCAST(UI*, player->inventory), ctx, transforms);
}