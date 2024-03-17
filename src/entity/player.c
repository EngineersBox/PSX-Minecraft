#include "player.h"

void playerInit(Player* player) {
    hotbarInit(&player->hotbar);
    inventoryInit(&player->inventory, &player->hotbar);
}

void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms) {
    inventoryRender(&player->inventory, ctx, transforms);
}