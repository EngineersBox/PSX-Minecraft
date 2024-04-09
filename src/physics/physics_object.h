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

typedef struct {
    // Fixed point block size (ONE * BLOCK_SIZE == single block)
    u32 jump_height;
    // Blocks per second
    u32 gravity;
    // Fixed point block size (ONE * BLOCK_SIZE == single block)
    u32 step_height;
} PhysicsObjectConfig;

/* Note that the following are booleans an note a single
 * enum field since it is possible to be in a combination
 * of these states at once if the AABB is bigger than a
 * single block.
 */
typedef struct {
    bool on_ground: 1;
    bool in_water: 1;
    bool in_lava: 1;
    bool in_web: 1;
    struct {
        bool horizontally: 1;
        bool vertically: 1;
        bool active: 1;
    } collision;
    bool no_clip;
    bool _pad: 24;
} PhysicsObjectFlags;

typedef struct {
    VECTOR position;
    VECTOR prev_position;
    VECTOR motion;
    VECTOR velocity;
    u32 y_size;
    u32 y_offset;
    u32 width;
    u32 height;
    u32 fall_distance;
    AABB aabb;
    PhysicsObjectConfig* config;
    PhysicsObjectFlags flags;
} PhysicsObject;

#define IPhysicsObject_IFACE \
    vfuncDefault(void, update, VSelf) \
    vfuncDefault(void, move, VSelf, u32 x, u32 y, u32 z) \
    vfuncDefault(void, moveWithHeading, VSelf)

void iPhysicsObjectUpdate(VSelf);
void IPhysicsObject_update(VSelf);

void iPhysicsObjectMove(VSelf, u32 x, u32 y, u32 z);
void IPhysicsObject_move(VSelf, u32 x, u32 y, u32 z);

void iPhysicsObjectMoveWithHeading(VSelf);
void IPhysicsObject_moveWithHeading(VSelf);

interface(IPhysicsObject);

#endif // PSX_MINECRAFT_PHYSICS_OBJECT_H
