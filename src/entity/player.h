#pragma once

#ifndef PSX_MINECRAFT_PLAYER_H
#define PSX_MINECRAFT_PLAYER_H

#include <psxgte.h>

#include "../core/camera.h"
#include "../game/gui/inventory.h"
#include "../game/gui/hotbar.h"
#include "../physics/physics_object.h"
#include "../core/input/input.h"
#include "entity.h"

extern const u32 player_collision_intervals_height[];
extern const u32 player_collision_intervals_radius[];
extern const PhysicsObjectConfig player_physics_object_config;

typedef struct {
    IEntity entity;
    ICamera* camera;
    PhysicsObject physics_object;
    VECTOR position;
    IUI inventory;
    IUI hotbar;
} Player;

// Forward declaration
typedef struct World World;

void playerInit(Player* player);
void playerDestroy(const Player* player);
void playerUpdate(Player* player, World* world);
void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms);

impl(IEntity, Player);

void playerRegisterInputHandler(VSelf, Input* input);
void Player_registerInputHandler(VSelf, Input* input);

impl(IInputHandler, Player);

#endif // PSX_MINECRAFT_PLAYER_H
