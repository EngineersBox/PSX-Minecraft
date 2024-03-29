#pragma once

#ifndef PSX_MINECRAFT_PHYSICS_OBJECT_H
#define PSX_MINECRAFT_PHYSICS_OBJECT_H

#include <psxgte.h>
#include <psxgpu.h>

// 9.81 * 4096 (ONE)
#define GRAVITY 40182

typedef struct {
    VECTOR position;
    VECTOR direction;
    VECTOR momentum;
} PhysicsObject;

void physicsUpdate(PhysicsObject* physobj);

#endif // PSX_MINECRAFT_PHYSICS_OBJECT_H
