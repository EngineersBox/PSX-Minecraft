#include "physics_object.h"

#include "../math/math_utils.h"
#include "../util/interface99_extensions.h"
#include "../game/blocks/block.h"

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
    if(flags->jumping) {
        if(flags->in_water || flags->in_lava) {
            self->motion.vy += 11468; // ONE_BLOCK * 0.04 = 11468
        } else if(flags->on_ground) {
            self->motion.vy += self->config->jump_height;
        }
    }
    self->move_strafe = fixedMulFrac(self->move_strafe, 280985); // ONE * 0.98 = 280985
    self->move_forward = fixedMulFrac(self->move_forward, 280985); // ONE * 0.98 = 280985
    iPhysicsObjectMoveWithHeading(self, world);
}

void iPhysicsObjectMoveWithHeading(VSelf, World* world) __attribute__((alias("IPhysicsObject_moveWithHeading")));
void IPhysicsObject_moveWithHeading(VSelf, World* world) {
    VSELF(PhysicsObject);
    i32 horizontal_shift = 260915; // ONE_BLOCK * 0.91 = 260915

    iPhysicsObjectMoveFlying(self, horizontal_shift);

    iPhysicsObjectMove(self, world, self->motion.vx, self->motion.vy, self->motion.vz);
    self->motion.vy -= 22937; // ONE * 0.08 = 2937
    self->motion.vy = fixedMulFrac(self->motion.vy, 280985); // ONE_BLOCK * 0.98 = 280985
    self->motion.vx = fixedMulFrac(self->motion.vx, horizontal_shift);
    self->motion.vz = fixedMulFrac(self->motion.vx, horizontal_shift);
}

bool collideWithWorld(PhysicsObject* physics_object, World* world, i32 motion_x, i32 motion_y, i32 motion_z) {
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
    applyMotion(y);
    applyMotion(z);
    VECTOR new_position = {};
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
        if (x_collision) {
            physics_object->flags.collided_horizontal = true;
            collision_detected = true;
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
        if (z_collision) {
            physics_object->flags.collided_horizontal = true;
            collision_detected = true;
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
        if (y_collision) {
            if (motion_y < 0) {
                new_position.vy = ONE_BLOCK;
                physics_object->flags.on_ground = true;
                physics_object->flags.jumping = false;
            }
            physics_object->velocity.vy = 0;
            collision_detected = true;
        } else {
            new_position.vy = motion_y;
        }
    }
    physics_object->position = vector_add(
        physics_object->position,
        new_position
    );
    if (physics_object->flags.on_ground) {
        const VECTOR below_position = (VECTOR) {
            .vx = physics_object->position.vx / ONE_BLOCK,
            .vy = (physics_object->position.vy - ONE) / ONE_BLOCK,
            .vz = physics_object->position.vz / ONE_BLOCK
        };
        const IBlock* iblock = worldGetBlock(world, &below_position);
        if (iblock == NULL) {
            return collision_detected;
        }
        const Block* block = VCAST_PTR(Block*, iblock);
        if (block->type != BLOCKTYPE_EMPTY) {
            physics_object->flags.on_ground = true;
        }
    }
    return collision_detected;
}

void iPhysicsObjectMove(VSelf, World* world, const i32 motion_x, const i32 motion_y, const i32 motion_z) __attribute__((alias("IPhysicsObject_move")));
void IPhysicsObject_move(VSelf, World* world, const i32 motion_x, const i32 motion_y, const i32 motion_z) {
    VSELF(PhysicsObject);
    if (self->flags.no_clip) {
        // aabbOffset(&self->aabb, motion_x, motion_y, motion_z);
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
    i32 dist = SquareRoot12(self->move_forward * self->move_forward + self->move_strafe * self->move_strafe);
    if (dist < 40) {
        return;
    } else if (dist < ONE) {
        dist = ONE_BLOCK;
    }
    dist = fixedDiv(horizontal_shift, dist);
    self->move_strafe = fixedMulFrac(self->move_strafe, dist);
    self->move_forward = fixedMulFrac(self->move_forward, dist);
    const i32 sin_yaw = isin(self->rotation.yaw);
    const i32 cos_yaw = icos(self->rotation.yaw);
    self->motion.vx += fixedMulFrac(self->move_strafe, cos_yaw) - fixedMulFrac(self->move_forward, sin_yaw);
    self->motion.vz += fixedMulFrac(self->move_forward, cos_yaw) + fixedMulFrac(self->move_strafe, sin_yaw);
    // float var5 = MathHelper.sin(this.rotationYaw * (float)Math.PI / 180.0F);
    // float var6 = MathHelper.cos(this.rotationYaw * (float)Math.PI / 180.0F);
    // this.motionX += var1 * var6 - var2 * var5;
    // this.motionZ += var2 * var6 + var1 * var5;
}
