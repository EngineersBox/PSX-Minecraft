#pragma once

#ifndef PSX_MINECRAFT_PLAYER_H
#define PSX_MINECRAFT_PLAYER_H

#include <stdint.h>
#include <psxgte.h>

#include "../core/camera.h"
#include "../game/gui/inventory.h"
#include "../game/gui/hotbar.h"

#define PLAYER_INV_NO_SLOT_AVAILABLE __UINT8_MAX__

typedef struct {
    Camera* camera;
    VECTOR position;
    uint8_t next_free_slot;
    IUI inventory;
    IUI hotbar;
} Player;

void playerInit(Player* player);
void playerDestroy(const Player* player);

void playerUpdate(Player* player);
void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_PLAYER_H
