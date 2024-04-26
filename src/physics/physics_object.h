#pragma once

#ifndef PSX_MINECRAFT_PHYSICS_OBJECT_H
#define PSX_MINECRAFT_PHYSICS_OBJECT_H

#include <psxgte.h>
#include <psxgpu.h>
#include <stdbool.h>
#include <interface99.h>

#include "../util/inttypes.h"
#include "../util/preprocessor.h"
#include "aabb.h"

#define ROTATION_SPEED 15
#define MINIMUM_VELOCITY 40

typedef struct {
    // Fixed point block size (ONE * BLOCK_SIZE == single block)
    u32 jump_height;
    // Blocks per second
    u32 gravity;
    // Fixed point block size (ONE * BLOCK_SIZE == single block)
    u32 step_height;
    // Height of the bounding box extending from the position Y
    u32 height;
    // Radius of the bounding box extending from the position X
    // and Z
    u32 radius;
    // Provide a set of descrete intervals to use when determining
    // collision with this physics object. The count is the length
    // of the two arrays. The arrays contain descrete intervals from
    // the inclusive ranges:
    // - [0, PhysicsObjectConfig::height]
    // - [0, PhysicsObjectConfig::radius]
    struct {
        u8 height_count;
        u8 radius_count;
        const u32* height;
        const u32* radius;
    } collision_intervals;
    // Offset from the minimum y value of the AABB to use as the
    // y position
    i32 y_offset;
} PhysicsObjectConfig;

/* Note that the following are booleans and not enum
 * entries since it is possible to be in a combination
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

typedef void (*PhysicsObjectFall)(VSelf, i32 distance, void* ctx);

typedef struct {
    const PhysicsObjectFall fall_handler;
} PhysicsObjectUpdateHandlers;

typedef struct {
    VECTOR position;
    VECTOR velocity;
    struct {
        // X-axis
        i32 pitch;
        // Y-axis
        i32 yaw;
    } rotation;
    struct {
        i16 forward;
        i16 strafe;
    } move;
    i32 fall_distance;
    i32 y_size;
    AABB aabb;
    const PhysicsObjectConfig* config;
    const PhysicsObjectUpdateHandlers* update_handlers;
    PhysicsObjectFlags flags;
} PhysicsObject;

// Forward declaration
typedef struct World World;

#define IPhysicsObject_IFACE \
    vfuncDefault(void, update, VSelf, World* world, void* ctx) \
    vfuncDefault(void, move, VSelf, World* world, i32 velocity_x, i32 velocity_y, i32 velocity_z, void* ctx) \
    vfuncDefault(void, moveWithHeading, VSelf, World* world, i32 move_strafe, i32 move_forward, void* ctx) \
    vfuncDefault(void, moveFlying, VSelf, i32 move_strafe, i32 move_forward, const i32 scaling) \

void iPhysicsObjectInit(PhysicsObject* physics_object, const PhysicsObjectConfig* config, const PhysicsObjectUpdateHandlers* update_handlers);
void iPhysicsObjectSetPosition(PhysicsObject* physics_object, const VECTOR* position);

void iPhysicsObjectUpdate(VSelf, World* world, void* ctx);
void IPhysicsObject_update(VSelf, World* world, void* ctx);

void iPhysicsObjectMoveWithHeading(VSelf, World* world, i32 move_strafe, i32 move_forward, void* ctx);
void IPhysicsObject_moveWithHeading(VSelf, World* world, i32 move_strafe, i32 move_forward, void* ctx);

void iPhysicsObjectMove(VSelf, World* world, i32 velocity_x, i32 velocity_y, i32 velocity_z, void* ctx);
void IPhysicsObject_move(VSelf, World* world, i32 velocity_x, i32 velocity_y, i32 velocity_z, void* ctx);

void iPhysicsObjectMoveFlying(VSelf, i32 move_strafe, i32 move_forward, const i32 scaling);
void IPhysicsObject_moveFlying(VSelf, i32 move_strafe, i32 move_forward, const i32 scaling);

interface(IPhysicsObject);

#endif // PSX_MINECRAFT_PHYSICS_OBJECT_H
