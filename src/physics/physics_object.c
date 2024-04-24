#include "physics_object.h"

#include <cvector_utils.h>

#include "../structure/cvector.h"
#include "../math/math_utils.h"
#include "../util/interface99_extensions.h"
#include "../game/blocks/blocks.h"
#include "../debug/debug.h"

// Forward declaration
IBlock* worldGetBlock(const World* world, const VECTOR* position);

void iPhysicsObjectInit(PhysicsObject* physics_object, const PhysicsObjectConfig* config) {
    physics_object->position = (VECTOR) {0},
    physics_object->rotation.pitch = 0;
    physics_object->rotation.yaw = 0;
    physics_object->velocity = (VECTOR) {0};
    physics_object->move.forward = 0;
    physics_object->move.strafe = 0;
    physics_object->fall_distance = 0;
    physics_object->aabb = (AABB) {0};
    physics_object->flags = (PhysicsObjectFlags) {0};
    physics_object->config = config;
    static const VECTOR _zero = {0};
    iPhysicsObjectSetPosition(physics_object, &_zero);
}

void iPhysicsObjectSetPosition(PhysicsObject* physics_object, const VECTOR* position) {
    physics_object->position = *position;
    physics_object->aabb = (AABB) {
        .min = (VECTOR) {
            .vx = position->vx - physics_object->config->radius,
            .vy = position->vy,
            .vz = position->vz - physics_object->config->radius
        },
        .max = (VECTOR) {
            .vx = position->vx + physics_object->config->radius,
            .vy = position->vy + physics_object->config->height,
            .vz = position->vz + physics_object->config->radius
        },
    };
}

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
            self->velocity.vy += 2867; // ONE_BLOCK * 0.04 = 2867
        } else if(flags->on_ground) {
            self->velocity.vy += self->config->jump_height;
        }
    }
    self->move.strafe = fixedMul(self->move.strafe, 4014); // ONE * 0.98 = 4014
    self->move.forward = fixedMul(self->move.forward, 4014); // ONE * 0.98 = 4014
    iPhysicsObjectMoveWithHeading(self, world);
}

i32 resolveGroundAcceleration(const PhysicsObject* physics_object,
                              const World* world,
                              i32 scaling) {
    if (!physics_object->flags.on_ground) {
        return scaling;
    }
    scaling = 2236; // ONE * 546.0 * 0.1 * 0.1 * 0.1
    VECTOR position = vector_const_div(physics_object->position, ONE_BLOCK);
    position.vy--;
    const IBlock* iblock = worldGetBlock(world, &position);
    if (iblock == NULL) {
        return scaling;
    }
    const Block* block = VCAST_PTR(Block*, iblock);
    if (block->id != BLOCKID_AIR || block->type != BLOCKTYPE_EMPTY) {
        scaling = block_attributes[block->id].slipperiness;
    }
    return scaling;
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
    i32 scaling;
    if (self->flags.on_ground) {
        scaling = fixedMul(
            resolveGroundAcceleration(
                self,
                world,
                BLOCK_DEFAULT_SLIPPERINESS
            ),
            409
        );
    } else {
        scaling = 81;
    }
    // Doesn't update position, only modifies velocity
    iPhysicsObjectMoveFlying(
        self,
        scaling
    );
    scaling = resolveGroundAcceleration(
        self,
        world,
        BLOCK_DEFAULT_SLIPPERINESS
    );
    VECTOR* velocity = &self->velocity;
    iPhysicsObjectMove(
        self,
        world,
        velocity->vx,
        velocity->vy,
        velocity->vz
    );
    if (!self->flags.on_ground) {
        // DEBUG_LOG("[PHYSICS] Velocity before: (%d,%d,%d)\n", inlineVecPtr(velocity));
        velocity->vy -= self->config->gravity;
        velocity->vy = fixedMul(velocity->vy, 4014); // ONE * 0.98 = 4014
        // DEBUG_LOG("[PHYSICS] Velocity after: (%d,%d,%d)\n", inlineVecPtr(velocity));
    }
    velocity->vx = absMinBound(fixedMul(velocity->vx, scaling), MINIMUM_VELOCITY, 0);
    velocity->vz = absMinBound(fixedMul(velocity->vz, scaling), MINIMUM_VELOCITY, 0);
}

/* NOTE: The aabb_v values used in each collide<axis> call can be transformed
 *       into units (i.e. div by ONE_BLOCK) before being passed to these methods.
 *       To do this, the min_<axis> values should be precomputed (div by ONE_BLOCK)
 *       and the collision_intervals in the same way. The problem is that we then
 *       loose precision with the checks for AABBs that are smaller than a block.
 *       Not sure if we really want to do this or not because of that.
 */

static bool collideX(const World* world, const PhysicsObjectConfig* config, const i32 x, const i32 min_y, const i32 min_z) {
    for (u8 yi = 0; yi < config->collision_intervals.height_count; yi++) {
        for (u8 zi = 0; zi <= config->collision_intervals.radius_count; zi++) {
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
                return true;
            }
        }
    }
    return false;
}

static bool collideZ(const World* world, const PhysicsObjectConfig* config, const i32 min_x, const i32 min_y, const i32 z) {
    for (u8 yi = 0; yi < config->collision_intervals.height_count; yi++) {
        for (u8 xi = 0; xi <= config->collision_intervals.radius_count; xi++) {
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
                return true;
            }
        }
    }
    return false;
}

static bool collideY(const World* world, const PhysicsObjectConfig* config, const i32 min_x, const i32 y, const i32 min_z) {
    for (u8 xi = 0; xi < config->collision_intervals.radius_count; xi++) {
        for (u8 zi = 0; zi <= config->collision_intervals.radius_count; zi++) {
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
            if (block->type != BLOCKTYPE_EMPTY) {
                return true;
            }
        }
    }
    return false;
}

#define deltaToBlockPos(pos, delta) ((delta) - positiveModulo(((pos) + (delta)), ONE_BLOCK))
#define deltaToBlockNeg(pos, delta) ((delta) + (ONE_BLOCK - positiveModulo(((pos) + (delta)), ONE_BLOCK)))

bool collideWithWorld(PhysicsObject* physics_object, const World* world, i32 velocity_x, i32 velocity_y, i32 velocity_z) {
    // DEBUG_LOG("[PHYSICS] Motion: (%d,%d,%d)\n", velocity_x, velocity_y, velocity_z);
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
    i32 x_diff_to_block = 0;
    i32 y_diff_to_block = 0;
    i32 z_diff_to_block = 0;
    bool collision_detected = false;
#define applyMotion(_v) \
    if (velocity_##_v < 0) { \
        _v##_diff_to_block = deltaToBlockNeg(min_##_v, velocity_##_v);  \
        DEBUG_LOG("[PHYSICS] " #_v " Motion: %d Diff: %d\n", velocity_##_v, _v##_diff_to_block); \
        _v = (min_##_v + velocity_##_v) / ONE_BLOCK; \
        test_##_v = true; \
    } else if (velocity_##_v > 0) { \
        _v##_diff_to_block = deltaToBlockPos(max_##_v, velocity_##_v); \
        DEBUG_LOG("[PHYSICS] " #_v " Motion: %d Diff: %d\n", velocity_##_v, _v##_diff_to_block); \
        _v = (max_##_v + velocity_##_v) / ONE_BLOCK; \
        test_##_v = true; \
    }
    applyMotion(x);
    applyMotion(y);
    applyMotion(z);
    VECTOR new_position = {0};
    if (test_x) {
        const bool x_collision = collideX(
            world,
            config,
            x,
            min_y,
            min_z
        );
        // DEBUG_LOG("[PHYSICS] Collision x: %s\n", x_collision ? "true" : "false");
        if (x_collision) {
            physics_object->flags.collided_horizontal = true;
            collision_detected = true;
            physics_object->velocity.vx = 0;
            new_position.vx = x_diff_to_block;
        } else {
            new_position.vx = velocity_x;
        }
    }
    if (test_z) {
        const bool z_collision = collideZ(
            world,
            config,
            min_x,
            min_y,
            z
        );
        // DEBUG_LOG("[PHYSICS] Collision z: %s\n", z_collision ? "true" : "false");
        if (z_collision) {
            physics_object->flags.collided_horizontal = true;
            collision_detected = true;
            physics_object->velocity.vz = 0;
            new_position.vz = z_diff_to_block;
        } else {
            new_position.vz = velocity_z;
        }
    }
    if (test_y) {
        const bool y_collision = collideY(
            world,
            config,
            min_x,
            y,
            min_z
        );
        // DEBUG_LOG("[PHYSICS] Collision y: %s\n", y_collision ? "true" : "false");
        if (y_collision) {
            if (velocity_y < 0) {
                physics_object->flags.on_ground = true;
                physics_object->flags.jumping = false;
                physics_object->fall_distance = 0;
            }
            physics_object->velocity.vy = 0;
            collision_detected = true;
            new_position.vy = y_diff_to_block;
        } else {
            new_position.vy = velocity_y;
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
        // DEBUG_LOG("[PHYSICS] Block below: (%d,%d,%d) => %s\n", inlineVec(position), blockGetName(block->id));
        if (block->type == BLOCKTYPE_EMPTY) {
            physics_object->flags.on_ground = false;
        }
    }
    return collision_detected;
}

cvector(AABB) getCollidingAABBs(const World* world, const AABB* aabb) {
    cvector(AABB) collided_aabbs = NULL;
    cvector_init(collided_aabbs, 0, NULL);
    const i32 min_x = fixedFloor(aabb->min.vx, ONE_BLOCK) / ONE_BLOCK;
    const i32 max_x = fixedFloor(aabb->max.vx + ONE_BLOCK, ONE_BLOCK) / ONE_BLOCK;
    const i32 min_y = fixedFloor(aabb->min.vy, ONE_BLOCK) / ONE_BLOCK;
    const i32 max_y = fixedFloor(aabb->max.vy + ONE_BLOCK, ONE_BLOCK) / ONE_BLOCK;
    const i32 min_z = fixedFloor(aabb->min.vz, ONE_BLOCK) / ONE_BLOCK;
    const i32 max_z = fixedFloor(aabb->max.vz + ONE_BLOCK, ONE_BLOCK) / ONE_BLOCK;
    // NOTE: ONE_BLOCK increments could be configurable in physics object config, might not be worth it though
    for (i32 x = min_x; x < max_x; x++) {
        for (i32 z = min_z; z < max_z; z++) {
            for (i32 y = min_y - 1; y < max_y; y++) {
                const VECTOR position = (VECTOR) {
                    .vx = x,
                    .vy = y,
                    .vz = z,
                };
                const IBlock* iblock = worldGetBlock(world, &position);
                if (iblock == NULL) {
                    continue;
                }
                const Block* block = VCAST_PTR(const Block*, iblock);
                if (block->type == BLOCKTYPE_EMPTY || block->id == BLOCKID_AIR) {
                    continue;
                }
                const AABB block_aabb = (AABB){
                    .min = (VECTOR) {
                        .vx = x * ONE_BLOCK,
                        .vy = y * ONE_BLOCK,
                        .vz = z * ONE_BLOCK,
                    },
                    .max = (VECTOR) {
                        .vx = (x * ONE_BLOCK) + ONE_BLOCK,
                        .vy = (y * ONE_BLOCK) + ONE_BLOCK,
                        .vz = (z * ONE_BLOCK) + ONE_BLOCK,
                    }
                };
                if (!aabbIntersects(aabb, &block_aabb)) {
                    continue;
                }
                cvector_push_back(collided_aabbs, block_aabb);
            }
        }
    }
    DEBUG_LOG("[PHYSICS] AABB collisions: %d\n", cvector_size(collided_aabbs));
    return collided_aabbs;
}

void updateFallState(PhysicsObject* physics_object, const i32 velocity_y) {
    if (physics_object->flags.on_ground) {
        if (physics_object->fall_distance > 0) {
            iPhysicsObjectFall(physics_object, physics_object->fall_distance);
            physics_object->fall_distance = 0;
        }
    } else {
        physics_object->fall_distance -= velocity_y;
    }
}

void _collideWithWorld(PhysicsObject* physics_object, const World* world, i32 vel_x, i32 vel_y, i32 vel_z) {
    const i32 curr_vel_x = vel_x;
    const i32 curr_vel_y = vel_y;
    const i32 curr_vel_z = vel_z;
    AABB aabb = (AABB) {0};
    aabbAddCoord(&physics_object->aabb, &aabb, vel_x, vel_y, vel_z);
    const cvector(AABB) collided_aabbs = getCollidingAABBs(world, &aabb);
    AABB* elem = NULL;
    cvector_for_each_in(elem, collided_aabbs) {
        vel_y = aabbYOffset(elem, &physics_object->aabb, vel_y);
    }
    aabbOffset(&physics_object->aabb, 0, vel_y, 0);
    cvector_for_each_in(elem, collided_aabbs) {
        vel_x = aabbXOffset(elem, &physics_object->aabb, vel_x);
    }
    aabbOffset(&physics_object->aabb, vel_x, 0, 0);
    cvector_for_each_in(elem, collided_aabbs) {
        vel_z = aabbZOffset(elem, &physics_object->aabb, vel_z);
    }
    aabbOffset(&physics_object->aabb, 0, 0, vel_z);
    // TODO: Position update here
    physics_object->position.vx = (physics_object->aabb.min.vx + physics_object->aabb.max.vx) >> 1;
    physics_object->position.vy = physics_object->aabb.min.vy;
    physics_object->position.vz = (physics_object->aabb.min.vz + physics_object->aabb.max.vz) >> 1;
    physics_object->flags.collided_horizontal = curr_vel_x != vel_x || curr_vel_z != vel_z;
    physics_object->flags.collided_vertical = curr_vel_y != vel_y;
    physics_object->flags.on_ground = curr_vel_y != vel_y && curr_vel_y < 0;
    physics_object->flags.collided = physics_object->flags.collided_horizontal || physics_object->flags.collided_vertical;
    updateFallState(physics_object, vel_y);
    if (curr_vel_x != vel_x) {
        physics_object->velocity.vx = 0;
    }
    if (curr_vel_y != vel_y) {
        physics_object->velocity.vy = 0;
    }
    if (curr_vel_z != vel_z) {
        physics_object->velocity.vz = 0;
    }
    // current_motion_x = this.posX - pos_x;
    // current_motion_y = this.posZ - pos_z;
    cvector_free(collided_aabbs);
}

void iPhysicsObjectMove(VSelf, World* world, const i32 velocity_x, const i32 velocity_y, const i32 velocity_z) __attribute__((alias("IPhysicsObject_move")));
void IPhysicsObject_move(VSelf, World* world, const i32 velocity_x, const i32 velocity_y, const i32 velocity_z) {
    VSELF(PhysicsObject);
    if (self->flags.no_clip) {
        self->position.vx += velocity_x;
        self->position.vy += velocity_y;
        self->position.vz += velocity_z;
        return;
    }
    _collideWithWorld(
        self,
        world,
        velocity_x,
        velocity_y,
        velocity_z
    );
    // Update fall state
    // updateFallState(self, velocity_y);
}

void iPhysicsObjectMoveFlying(VSelf, const i32 scaling) __attribute__((alias("IPhysicsObject_moveFlying")));
void IPhysicsObject_moveFlying(VSelf, const i32 scaling) {
    VSELF(PhysicsObject);
    i32 dist = SquareRoot12(
        fixedMul(self->move.forward, self->move.forward)
        + fixedMul(self->move.strafe, self->move.strafe)
    );
    if (dist < 40) { // ONE * 0.01
        return;
    } else if (dist < ONE) {
        dist = ONE;
    }
    dist = (scaling << 12) / (dist >> 12);
    self->move.strafe = fixedMul(self->move.strafe, dist);
    self->move.forward = fixedMul(self->move.forward, dist);
    const i32 sin_yaw = isin(self->rotation.yaw >> FIXED_POINT_SHIFT);
    const i32 cos_yaw = icos(self->rotation.yaw >> FIXED_POINT_SHIFT);
    self->velocity.vx += fixedMul(self->move.strafe, cos_yaw) - fixedMul(self->move.forward, sin_yaw);
    self->velocity.vz += fixedMul(self->move.forward, cos_yaw) + fixedMul(self->move.strafe, sin_yaw);
}

void iPhysicsObjectFall(VSelf, i32 distance) __attribute__((alias("IPhysicsObject_fall")));
void IPhysicsObject_fall(VSelf, i32 distance) {
    // Do nothing by default
}
