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
    IUI inventory;
    IUI hotbar;
} Player;

void playerInit(Player* player);
void playerDestroy(const Player* player);

void playerUpdate(Player* player);
void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms);

typedef enum {
    // Added and freed iitem instance
    PLAYER_STORE_RESULT_ADDED_ALL = 0,
    // Added some of stack, didn't free iitem, updated stack_size
    PLAYER_STORE_RESULT_ADDED_SOME,
    // No space in inventory, didn't free iitem, didn't update stack size
    PLAYER_STORE_RESULT_NO_SPACE,
    // Added to new slot, didn't free iitem
    PLAYER_STORE_RESULT_ADDED_NEW_SLOT
} PlayerStoreResult;

PlayerStoreResult playerStoreItem(Player* player, IItem* iitem);

#endif // PSX_MINECRAFT_PLAYER_H
