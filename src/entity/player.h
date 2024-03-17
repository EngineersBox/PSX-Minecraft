#pragma once

#ifndef PSX_MINECRAFT_PLAYER_H
#define PSX_MINECRAFT_PLAYER_H

#include <psxgte.h>

#include "../core/camera.h"
#include "../game/gui/inventory.h"
#include "../game/gui/hotbar.h"

typedef struct {
    Camera camera;
    VECTOR position;
    Inventory inventory;
    Hotbar hotbar;
} Player;

void playerInit(Player* player);
void playerUpdate(Player* player);
void playerRender(Player* player, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_PLAYER_H
