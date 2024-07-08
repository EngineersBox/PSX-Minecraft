#pragma once

#ifndef _PSX_MINECRAFT__GAME_WORLD__WORLD_RAYCAST_H_
#define _PSX_MINECRAFT__GAME_WORLD__WORLD_RAYCAST_H_

#include <psxgte.h>

#include "../../util/inttypes.h"
#include "../../core/camera.h"
#include "../blocks/block.h"

#define ALLOW_RAYCAST_OUTSIDE_WORLD true

// Fordward declaration
typedef struct World World;

typedef struct {
    VECTOR pos;
    IBlock* block;
    VECTOR face;
} RayCastResult;

RayCastResult worldRayCastIntersection(const World* world, const Camera* camera, i32 radius);

#endif // _PSX_MINECRAFT__GAME_WORLD__WORLD_RAYCAST_H_
