#pragma once

#ifndef PSX_MINECRAFT_PLAYER_H
#define PSX_MINECRAFT_PLAYER_H

#include <psxgte.h>
#include <stdbool.h>

#include "../core/camera.h"
#include "../game/gui/inventory.h"
#include "../game/gui/hotbar.h"
#include "../physics/physics_object.h"
#include "../core/input/input.h"
#include "../game/blocks/breaking_state.h"
#include "entity.h"

extern const u32 player_collision_intervals_height[];
extern const u32 player_collision_intervals_radius[];
extern const PhysicsObjectConfig player_physics_object_config;
extern const PhysicsObjectUpdateHandlers player_physics_object_update_handlers;

// 10 hearts with 2 points in each heart
#define PLAYER_MAX_HEALTH 20
#define PLAYER_REACH_DISTANCE 6
// ONE_BLOCK * 1.7
#define PLAYER_CAMERA_OFFSET 487424

typedef struct {
    Entity entity;
    ICamera* camera;
    PhysicsObject physics_object;
    IUI inventory;
    IUI hotbar;
    BreakingState breaking;
} Player;

// Forward declaration
typedef struct World World;

typedef struct {
    Player* player;
    World* world;
} PlayerInputHandlerContext;

void playerInit(Player* player);
void playerDestroy(const Player* player);
void playerUpdate(Player* player, World* world);
void playerUpdateCamera(const Player* player);
void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms);

void playerFallHandler(PhysicsObject* physics_object, i32 distance, void* ctx);

impl(IEntity, Player);

void playerRegisterInputHandler(VSelf, Input* input, void* ctx);
void Player_registerInputHandler(VSelf, Input* input, void* ctx);

impl(IInputHandler, Player);

#endif // PSX_MINECRAFT_PLAYER_H
