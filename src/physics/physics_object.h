#pragma once

#ifndef PSX_MINECRAFT_PHYSICS_OBJECT_H
#define PSX_MINECRAFT_PHYSICS_OBJECT_H

#include <psxgte.h>
#include <psxgpu.h>
#include <stdbool.h>

#include "aabb.h"
#include "../util/inttypes.h"
#include "../util/preprocessor.h"

typedef struct {
    VECTOR position;
    VECTOR prev_position;
    VECTOR motion;
    VECTOR velocity;
    /* Note that the following are booleans an note a single
     * enum field since it is possible to be in a combination
     * of these states at once if the AABB is bigger than a
     * single block.
     */
    bool on_ground;
    bool in_water;
    bool in_lava;
    bool in_web;
    struct {
        bool horizontally;
        bool vertically;
        bool active;
    } collision;
    u32 width;
    u32 height;
    u32 fall_distance;
    u32 ticks_passed;
    AABB aabb;
} PhysicsObject;

void physicsUpdate(PhysicsObject* physics_object);
void physicsJump(PhysicsObject* physics_object);
void physicsMove(PhysicsObject* physics_object);
void physicsMoveWithHeading(PhysicsObject* physics_object);

#endif // PSX_MINECRAFT_PHYSICS_OBJECT_H
