#pragma once

#ifndef PSX_MINECRAFT_PLAYER_H
#define PSX_MINECRAFT_PLAYER_H

#include <psxgte.h>

#include "../core/camera.h"
#include "../game/inventory/inventory.h"

typedef struct {
    Camera camera;
    VECTOR position;
    Inventory* inventory;
} Player;

void playerUpdate(Player* player);

#endif // PSX_MINECRAFT_PLAYER_H
