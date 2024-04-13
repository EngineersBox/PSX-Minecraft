#pragma once

#ifndef PSX_MINECRAFT_PHYSICS_OBJECT_H
#define PSX_MINECRAFT_PHYSICS_OBJECT_H

#include <psxgte.h>
#include <psxgpu.h>
#include <stdbool.h>
#include <interface99.h>

#include "aabb.h"
#include "../util/inttypes.h"
#include "../util/preprocessor.h"
#include "../game/world/world.h"

typedef struct {
    // Fixed point block size (ONE * BLOCK_SIZE == single block)
    u32 jump_height;
    // Blocks per second
    u32 gravity;
    // Fixed point block size (ONE * BLOCK_SIZE == single block)
    u32 step_height;
    i16 height;
    i16 radius;
} PhysicsObjectConfig;

/* Note that the following are booleans an note a single
 * enum field since it is possible to be in a combination
 * of these states at once if the AABB is larger than a
 * single block.
 */
typedef struct {
    bool on_ground: 1;
    bool in_water: 1;
    bool in_lava: 1;
    bool in_web: 1;
    bool collided_horizontal: 1;
    bool collided_vertical: 1;
    bool collided: 1;
    bool jumping: 1;
    bool sneaking: 1;
    bool no_clip: 1;
    u32 _pad: 22;
} PhysicsObjectFlags;

typedef struct {
    VECTOR position;
    VECTOR motion;
    VECTOR velocity;
    i16 move_forward;
    i16 move_strafe;
    PhysicsObjectConfig* config;
    PhysicsObjectFlags flags;
} PhysicsObject;

#define IPhysicsObject_IFACE \
    vfuncDefault(void, setPosition, VSelf, i32 x, i32 y, i32 z) \
    vfuncDefault(void, update, VSelf, World* world) \
    vfuncDefault(void, move, VSelf, World* world, i32 motion_x, i32 motion_y, i32 motion_z) \
    vfuncDefault(void, moveWithHeading, VSelf, World* world)

void iPhysicsObjectUpdate(VSelf, World* world);
void IPhysicsObject_update(VSelf, World* world);

void iPhysicsObjectMoveWithHeading(VSelf, World* world);
void IPhysicsObject_moveWithHeading(VSelf, World* world);

void iPhysicsObjectMove(VSelf, World* world, i32 motion_x, i32 motion_y, i32 motion_z);
void IPhysicsObject_move(VSelf, World* world, i32 motion_x, i32 motion_y, i32 motion_z);

interface(IPhysicsObject);

#endif // PSX_MINECRAFT_PHYSICS_OBJECT_H
