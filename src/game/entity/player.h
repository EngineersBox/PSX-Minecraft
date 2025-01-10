#pragma once

#ifndef PSXMC_PLAYER_H
#define PSXMC_PLAYER_H

#include <psxgte.h>
#include <stdbool.h>

#include "../../core/camera.h"
#include "../../core/input/input.h"
#include "../blocks/breaking_state.h"
#include "../gui/inventory.h"
#include "../gui/hotbar.h"
#include "entity.h"

extern const i32 player_collision_intervals_height[];
extern const i32 player_collision_intervals_radius[];
extern const PhysicsObjectConfig player_physics_object_config;
extern const PhysicsObjectUpdateHandlers player_physics_object_update_handlers;

// 10 hearts with 2 points in each heart
#define PLAYER_MAX_HEALTH 20
#define PLAYER_REACH_DISTANCE 6
// ONE_BLOCK * 1.7
#define PLAYER_CAMERA_OFFSET 487424

typedef struct {
    Entity entity;
    Camera* camera;
    IUI inventory;
    IUI hotbar;
    BreakingState breaking;
} Player;

extern IEntity player_entity;
extern Player* player;

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

#define Player_damage_CUSTOM ()
void playerDamage(VSelf, const i16 amount);
void Player_damage(VSelf, const i16 amount);

impl(IEntity, Player);

void playerRegisterInputHandler(VSelf, Input* input, void* ctx);
void Player_registerInputHandler(VSelf, Input* input, void* ctx);

impl(IInputHandler, Player);

#endif // PSXMC_PLAYER_H
