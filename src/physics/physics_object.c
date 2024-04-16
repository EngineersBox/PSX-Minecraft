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
            self->motion.vy += 2867; // ONE_BLOCK * 0.04 = 2867
        } else if(flags->on_ground) {
            self->motion.vy += self->config->jump_height;
        }
    }
    self->move_strafe = fixedMul(self->move_strafe, 4014); // ONE * 0.98 = 4014
    self->move_forward = fixedMul(self->move_forward, 4014); // ONE * 0.98 = 4014
    iPhysicsObjectMoveWithHeading(self, world);
}

i32 resolveGroundAcceleration(const PhysicsObject* physics_object,
                              const World* world,
                              i32 horizontal_acceleration) {
    if (!physics_object->flags.on_ground) {
        return horizontal_acceleration;
    }
    horizontal_acceleration = 2236; // ONE * 546.0 * 0.1 * 0.1 * 0.1
    VECTOR position = vector_const_div(physics_object->position, ONE_BLOCK);
    position.vy--;
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
    // ONE * 0.91 =  3727
    // ONE * 0.16277136 = 666
    // ONE * 0.1 = 409
    // ONE * 0.02 = 81
    // 0.16277136 / (0.1 * 0.1 * 0.1) = 162.77136
    // ((666 << 12) / ((((409 * 409) >> 12) * 409) >> 12)) / (1 << 12) = 222 (inaccuracy here)
    i32 shift;
    if (self->flags.on_ground) {
        shift = fixedMul(
            resolveGroundAcceleration(
                self,
                world,
                3727
            ),
            409
        );
    } else {
        shift = 81;
    }
    iPhysicsObjectMoveFlying(
        self,
        shift
    );
    const i32 horizontal_acceleration = resolveGroundAcceleration(
        self,
        world,
        3727
    );
    iPhysicsObjectMove(
        self,
        world,
        self->motion.vx,
        self->motion.vy,
        self->motion.vz
    );
    if (!self->flags.on_ground) {
        self->motion.vy -= self->config->gravity;
        self->motion.vy = fixedMul(self->motion.vy, 4014); // ONE * 0.98 = 4014
    }
    self->motion.vx = fixedMul(self->motion.vx, horizontal_acceleration);
    self->motion.vz = fixedMul(self->motion.vz, horizontal_acceleration);
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
        _v = (min_##_v + motion_##_v) / ONE_BLOCK; \
        test_##_v = true; \
    } else if (motion_##_v > 0) { \
        _v = (max_##_v + motion_##_v) / ONE_BLOCK; \
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
        for (u8 yi = 0; yi < config->collision_intervals.height_count && !x_collision; yi++) {
            for (u8 zi = 0; zi <= config->collision_intervals.radius_count && !x_collision; zi++) {
                const i32 aabb_y = min_y + config->collision_intervals.height[yi];
                const i32 aabb_z = min_z + config->collision_intervals.radius[zi];
                const VECTOR position = (VECTOR) {
                    .vx = x,
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
        DEBUG_LOG("[PHYSICS] Collision x: %s\n", x_collision ? "true" : "false");
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
        for (u8 yi = 0; yi < config->collision_intervals.height_count && !z_collision; yi++) {
            for (u8 xi = 0; xi <= config->collision_intervals.radius_count && !z_collision; xi++) {
                const i32 aabb_y = min_y + config->collision_intervals.height[yi];
                const i32 aabb_x = min_x + config->collision_intervals.radius[xi];
                const VECTOR position = (VECTOR) {
                    .vx = aabb_x / ONE_BLOCK,
                    .vy = aabb_y / ONE_BLOCK,
                    .vz = z
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
        DEBUG_LOG("[PHYSICS] Collision z: %s\n", z_collision ? "true" : "false");
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
        for (u8 xi = 0; xi < config->collision_intervals.radius_count && !y_collision; xi++) {
            for (u8 zi = 0; zi <= config->collision_intervals.radius_count && !y_collision; zi++) {
                const i32 aabb_x = min_x + config->collision_intervals.radius[xi];
                const i32 aabb_z = min_z + config->collision_intervals.radius[zi];
                const VECTOR position = (VECTOR) {
                    .vx = aabb_x / ONE_BLOCK,
                    .vy = y,
                    .vz = aabb_z / ONE_BLOCK
                };
                const IBlock* iblock = worldGetBlock(world, &position);
                if (iblock == NULL) {
                    continue;
                }
                const Block* block = VCAST_PTR(Block*, iblock);
                // DEBUG_LOG(
                //     "[PHYSICS] Block: %s, (%d,%d,%d)\n",
                //     block_attributes[block->id].name,
                //     inlineVec(position)
                // );
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
    // Check if we are on the ground in the new position after moving, if not
    // then we need to update the on_ground flag so that we apply gravity to
    // the object after this call returns
    if (physics_object->flags.on_ground) {
        VECTOR position = vector_const_div(physics_object->position, ONE_BLOCK);
        position.vy--;
        const IBlock* iblock = worldGetBlock(world, &position);
        if (iblock == NULL) {
            return collision_detected;
        }
        const Block* block = VCAST_PTR(Block*, iblock);
        if (block->type != BLOCKTYPE_EMPTY) {
            physics_object->flags.on_ground = false;
        }
    }
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
        fixedMul(self->move_forward, self->move_forward)
        + fixedMul(self->move_strafe, self->move_strafe)
    );
    if (dist < 40) { // ONE * 0.01
        return;
    } else if (dist < ONE) {
        dist = ONE;
    }
    dist = (horizontal_shift << 12) / (dist >> 12);
    self->move_strafe = fixedMul(self->move_strafe, dist);
    self->move_forward = fixedMul(self->move_forward, dist);
    const i32 sin_yaw = isin(self->rotation.yaw >> FIXED_POINT_SHIFT);
    const i32 cos_yaw = icos(self->rotation.yaw >> FIXED_POINT_SHIFT);
    self->motion.vx += fixedMul(self->move_strafe, cos_yaw) - fixedMul(self->move_forward, sin_yaw);
    self->motion.vz += fixedMul(self->move_forward, cos_yaw) + fixedMul(self->move_strafe, sin_yaw);
}
