#include "physics_object.h"

#include "../math/math_utils.h"
#include "../util/interface99_extensions.h"
#include "../game/blocks/blocks.h"
#include "../debug/debug.h"

// Forward declaration
IBlock* worldGetBlock(const World* world, const VECTOR* position);

/* Order:
 * update(...) { ... moveWithHeading(...) ... }
 * moveWithHeading(...) { ... move(...) ... }
 * move(...) { ... }
 */

void iPhysicsObjectUpdate(VSelf, World* world) __attribute__((alias("IPhysicsObject_update")));
void IPhysicsObject_update(VSelf, World* world) {
    VSELF(PhysicsObject);
    const PhysicsObjectFlags* flags = &self->flags;
    if (flags->jumping) {
        if(flags->in_water || flags->in_lava) {
            self->motion.vy += 11468; // ONE_BLOCK * 0.04 = 11468
        } else if(flags->on_ground) {
            self->motion.vy += self->config->jump_height;
        }
    }
    self->move_strafe = fixedMulFrac(self->move_strafe, 4014); // ONE * 0.98 = 4014
    self->move_forward = fixedMulFrac(self->move_forward, 4014); // ONE * 0.98 = 4014
    iPhysicsObjectMoveWithHeading(self, world);
}

i32 resolveGroundAcceleration(const PhysicsObject* physics_object,
                              const World* world,
                              i32 horizontal_acceleration) {
    if (!physics_object->flags.on_ground) {
        return horizontal_acceleration;
    }
    horizontal_acceleration = 1867; // ONE_BLOCK * 546.0F * 0.1F * 0.1F * 0.1F;
    const VECTOR position = (VECTOR) {
        .vx = physics_object->position.vx / ONE_BLOCK,
        .vy = (physics_object->position.vy - ONE_BLOCK) / ONE_BLOCK,
        .vz = physics_object->position.vz / ONE_BLOCK,
    };
    const IBlock* iblock = worldGetBlock(world, &position);
    if (iblock == NULL) {
        return horizontal_acceleration;
    }
    const Block* block = VCAST_PTR(Block*, iblock);
    if (block->id != BLOCKID_AIR || block->type != BLOCKTYPE_EMPTY) {
        horizontal_acceleration = block_attributes[block->id].slipperiness;
    }
    return horizontal_acceleration;
}

void iPhysicsObjectMoveWithHeading(VSelf, World* world) __attribute__((alias("IPhysicsObject_moveWithHeading")));
void IPhysicsObject_moveWithHeading(VSelf, World* world) {
    VSELF(PhysicsObject);
    i32 horizontal_acceleration = resolveGroundAcceleration(
        self,
        world,
        143360
    );
    // ONE_BLOCK * 0.91 = 143360
    // ONE_BLOCK * 0.16277136 = 46669
    // ONE_BLOCK * 0.1 = 28672
    const i32 shift = fixedMulFrac(
        28672,
        fixedMulFrac(
            46669,
            fixedMulFrac(
                horizontal_acceleration,
                fixedMulFrac(
                    horizontal_acceleration,
                    horizontal_acceleration
                )
            )
        )
    );
    iPhysicsObjectMoveFlying(
        self,
        shift
    );
    horizontal_acceleration = resolveGroundAcceleration(
        self,
        world,
        143360
    );
    iPhysicsObjectMove(self, world, self->motion.vx, self->motion.vy, self->motion.vz);
    if (!self->flags.on_ground) {
        self->motion.vy -= self->config->gravity;
        self->motion.vy = fixedMulFrac(self->motion.vy, 4014); // ONE * 0.98 = 4014
    }
    self->motion.vx = fixedMulFrac(self->motion.vx, horizontal_acceleration);
    self->motion.vz = fixedMulFrac(self->motion.vz, horizontal_acceleration);
}

bool collideWithWorld(PhysicsObject* physics_object, World* world, i32 motion_x, i32 motion_y, i32 motion_z) {
    // DEBUG_LOG("[PHYSICS] Motion: (%d,%d,%d)\n", motion_x, motion_y, motion_z);
    const PhysicsObjectConfig* config = physics_object->config;
    const i32 min_x = physics_object->position.vx - config->radius;
    const i32 min_y = physics_object->position.vy;
    const i32 min_z = physics_object->position.vz - config->radius;
    const i32 max_x = physics_object->position.vx + config->radius;
    const i32 max_y = physics_object->position.vy + config->height;
    const i32 max_z = physics_object->position.vz + config->radius;
    // https://github.com/camthesaxman/cubecraft/blob/1dd4f9f25069bfaba0ac659c845c5eccbea4c08a/source/field.c#L318
    bool test_x = false;
    bool test_y = false;
    bool test_z = false;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    bool collision_detected = false;
#define applyMotion(_v) \
    if (motion_##_v < 0) { \
        _v = min_##_v + motion_##_v; \
        test_##_v = true; \
    } else if (motion_##_v > 0) { \
        _v = max_##_v + motion_##_v; \
        test_##_v = true; \
    }
    applyMotion(x);
    // DEBUG_LOG("[PHYSICS] Test x: %s\n", test_x ? "true" : "false");
    applyMotion(y);
    // DEBUG_LOG("[PHYSICS] Test y: %s\n", test_y ? "true" : "false");
    applyMotion(z);
    // DEBUG_LOG("[PHYSICS] Test z: %s\n", test_z ? "true" : "false");
    VECTOR new_position = {0};
    if (test_x) {
        bool x_collision = false;
        for (i32 aabb_y = min_y; aabb_y <= max_y && !x_collision; aabb_y += ONE) {
            for (i32 aabb_z = min_z; aabb_z <= max_z && !x_collision; aabb_z += ONE) {
                const VECTOR position = (VECTOR) {
                    .vx = x / ONE_BLOCK,
                    .vy = aabb_y / ONE_BLOCK,
                    .vz = aabb_z / ONE_BLOCK
                };
                const IBlock* iblock = worldGetBlock(world, &position);
                if (iblock == NULL) {
                    continue;
                }
                const Block* block = VCAST_PTR(Block*, iblock);
                if (block->type != BLOCKTYPE_EMPTY) {
                    x_collision = true;
                }
            }
        }
        // DEBUG_LOG("[PHYSICS] Collision x: %s\n", x_collision ? "true" : "false");
        if (x_collision) {
            physics_object->flags.collided_horizontal = true;
            collision_detected = true;
            physics_object->motion.vx = 0;
        } else {
            new_position.vx = motion_x;
        }
    }
    if (test_z) {
        bool z_collision = false;
        for (i32 aabb_y = min_y; aabb_y <= max_y && !z_collision; aabb_y += ONE) {
            for (i32 aabb_x = min_x; aabb_x <= max_x && !z_collision; aabb_x += ONE) {
                const VECTOR position = (VECTOR) {
                    .vx = aabb_x / ONE_BLOCK,
                    .vy = aabb_y / ONE_BLOCK,
                    .vz = z / ONE_BLOCK
                };
                const IBlock* iblock = worldGetBlock(world, &position);
                if (iblock == NULL) {
                    continue;
                }
                const Block* block = VCAST_PTR(Block*, iblock);
                if (block->type != BLOCKTYPE_EMPTY) {
                    z_collision = true;
                }
            }
        }
        // DEBUG_LOG("[PHYSICS] Collision z: %s\n", z_collision ? "true" : "false");
        if (z_collision) {
            physics_object->flags.collided_horizontal = true;
            collision_detected = true;
            physics_object->motion.vz = 0;
        } else {
            new_position.vz = motion_z;
        }
    }
    if (test_y) {
        bool y_collision = false;
        for (i32 aabb_x = min_x; aabb_x <= max_x && !y_collision; aabb_x += ONE) {
            for (i32 aabb_z = min_z; aabb_z <= max_z && !y_collision; aabb_z += ONE) {
                const VECTOR position = (VECTOR) {
                    .vx = aabb_x / ONE_BLOCK,
                    .vy = y / ONE_BLOCK,
                    .vz = aabb_z / ONE_BLOCK
                };
                const IBlock* iblock = worldGetBlock(world, &position);
                if (iblock == NULL) {
                    continue;
                }
                const Block* block = VCAST_PTR(Block*, iblock);
                if (block->type != BLOCKTYPE_EMPTY) {
                    y_collision = true;
                }
            }
        }
        // DEBUG_LOG("[PHYSICS] Collision y: %s\n", y_collision ? "true" : "false");
        if (y_collision) {
            if (motion_y < 0) {
                new_position.vy = ONE_BLOCK;
                physics_object->flags.on_ground = true;
                physics_object->flags.jumping = false;
            }
            physics_object->motion.vy = 0;
            collision_detected = true;
        } else {
            new_position.vy = motion_y;
        }
    }
    physics_object->position = vector_add(
        physics_object->position,
        new_position
    );
    return collision_detected;
}

void iPhysicsObjectMove(VSelf, World* world, const i32 motion_x, const i32 motion_y, const i32 motion_z) __attribute__((alias("IPhysicsObject_move")));
void IPhysicsObject_move(VSelf, World* world, const i32 motion_x, const i32 motion_y, const i32 motion_z) {
    VSELF(PhysicsObject);
    if (self->flags.no_clip) {
        self->position.vx += motion_x;
        self->position.vy += motion_y;
        self->position.vz += motion_z;
        return;
    }
    collideWithWorld(
        self,
        world,
        motion_x,
        motion_y,
        motion_z
    );
}

void iPhysicsObjectMoveFlying(VSelf, i32 horizontal_shift) __attribute__((alias("IPhysicsObject_moveFlying")));
void IPhysicsObject_moveFlying(VSelf, i32 horizontal_shift) {
    VSELF(PhysicsObject);
    i32 dist = SquareRoot12(
        fixedMulFrac(self->move_forward, self->move_forward)
        + fixedMulFrac(self->move_strafe, self->move_strafe)
    );
    if (dist < 40) {
        return;
    } else if (dist < ONE) {
        dist = ONE;
    }
    dist = (horizontal_shift << 12) / dist;
    self->move_strafe = fixedMulFrac(self->move_strafe, dist);
    self->move_forward = fixedMulFrac(self->move_forward, dist);
    const i32 sin_yaw = isin(self->rotation.yaw >> FIXED_POINT_SHIFT);
    const i32 cos_yaw = icos(self->rotation.yaw >> FIXED_POINT_SHIFT);
    self->motion.vx += fixedMulFrac(self->move_strafe, cos_yaw) - fixedMulFrac(self->move_forward, sin_yaw);
    self->motion.vz += fixedMulFrac(self->move_forward, cos_yaw) + fixedMulFrac(self->move_strafe, sin_yaw);
}
