#pragma once

#ifndef PSX_MINECRAFT_PHYSICS_OBJECT_H
#define PSX_MINECRAFT_PHYSICS_OBJECT_H

#include <psxgte.h>
#include <psxgpu.h>

#include "aabb.h"

#ifndef GRAVITY
// 9.81 * 4096 (ONE)
#define GRAVITY 40182
#endif

typedef struct {
    VECTOR position;
    VECTOR acceleration;
    VECTOR velocity;
    uint16_t ticks_passed;
    uint16_t mass;
    AABB aabb;
} PhysicsObject;

void physicsUpdate(PhysicsObject* physobj);

#endif // PSX_MINECRAFT_PHYSICS_OBJECT_H
