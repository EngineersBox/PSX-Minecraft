#include "physics_object.h"

#include <cvector_utils.h>

#include "../structure/cvector.h"
#include "../math/math_utils.h"
#include "../util/interface99_extensions.h"
#include "../game/blocks/blocks.h"
#include "../debug/debug.h"

// Forward declaration
IBlock* worldGetBlock(const World* world, const VECTOR* position);

void iPhysicsObjectInit(PhysicsObject* physics_object,
                        const PhysicsObjectConfig* config,
                        const PhysicsObjectUpdateHandlers* update_handlers) {
    if (config == NULL) {
        printf("[ERROR] Physics object must have a config supplied\n");
        abort();
        return;
    } else if (update_handlers == NULL) {
        printf("[ERROR] Physics object must have update handlers supplied\n");
        abort();
        return;
    }
    physics_object->rotation.pitch = 0;
    physics_object->rotation.yaw = 0;
    physics_object->velocity = (VECTOR) {0};
    physics_object->move.forward = 0;
    physics_object->move.strafe = 0;
    physics_object->fall_distance = 0;
    physics_object->aabb = (AABB) {0};
    physics_object->flags = (PhysicsObjectFlags) {0};
    physics_object->config = config;
    physics_object->update_handlers = update_handlers;
    static const VECTOR _zero = {0};
    iPhysicsObjectSetPosition(physics_object, &_zero);
}

void iPhysicsObjectSetPosition(PhysicsObject* physics_object, const VECTOR* position) {
    physics_object->position = *position;
    physics_object->aabb = (AABB) {
        .min = (VECTOR) {
            .vx = position->vx - physics_object->config->radius,
            .vy = position->vy - physics_object->config->y_offset + physics_object->y_size,
            .vz = position->vz - physics_object->config->radius
        },
        .max = (VECTOR) {
            .vx = position->vx + physics_object->config->radius,
            .vy = position->vy - physics_object->config->y_offset + physics_object->y_size + physics_object->config->height,
            .vz = position->vz + physics_object->config->radius
        },
    };
}

/* Order:
 * update(...) { ... moveWithHeading(...) ... }
 * moveWithHeading(...) { ... move(...) ... }
 * move(...) { ... }
 */

void iPhysicsObjectUpdate(VSelf, World* world, void* ctx) __attribute__((alias("IPhysicsObject_update")));
void IPhysicsObject_update(VSelf, World* world, void* ctx) {
    VSELF(PhysicsObject);
    const PhysicsObjectFlags* flags = &self->flags;
    if (flags->jumping) {
        if (flags->in_water || flags->in_lava) {
            self->velocity.vy += 2867; // ONE_BLOCK * 0.04 = 2867
        } else if (flags->on_ground) {
            self->velocity.vy += self->config->jump_height;
        }
    }
    self->move.strafe = fixedMul(self->move.strafe, 4014); // ONE * 0.98 = 4014
    self->move.forward = fixedMul(self->move.forward, 4014); // ONE * 0.98 = 4014
    iPhysicsObjectMoveWithHeading(self, world, self->move.strafe, self->move.forward, ctx);
}

i32 resolveGroundAcceleration(const PhysicsObject* physics_object,
                              const World* world,
                              i32 scaling) {
    if (!physics_object->flags.on_ground) {
        return scaling;
    }
    // scaling = 156549; // ONE_BLOCK * 546.0 * 0.1 * 0.1 * 0.1
    scaling = 2236; // ONE * 546.0 * 0.1 * 0.1 * 0.1
    VECTOR position = (VECTOR) {
        .vx = fixedFloor(physics_object->position.vx, ONE_BLOCK) / ONE_BLOCK,
        .vy = fixedFloor(physics_object->aabb.min.vy, ONE_BLOCK) / ONE_BLOCK,
        .vz = fixedFloor(physics_object->position.vz, ONE_BLOCK) / ONE_BLOCK,
    };
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

void iPhysicsObjectMoveWithHeading(VSelf, World* world, i32 move_strafe, i32 move_forward, void* ctx) __attribute__((alias("IPhysicsObject_moveWithHeading")));
void IPhysicsObject_moveWithHeading(VSelf, World* world, i32 move_strafe, i32 move_forward, void* ctx) {
    VSELF(PhysicsObject);
    // ONE * 0.91 = 3727
    // ONE * 0.16277136 = 666
    // ONE * 0.1 = 409
    // ONE * 0.02 = 81
    // 0.16277136 / (0.1 * 0.1 * 0.1) = 162.77136
    // ((666 << 12) / ((((409 * 409) >> 12) * 409) >> 12)) / (1 << 12) = 222 (inaccuracy here)

    // This determines the amount of movement applied at this tick
    // essentially implying movement speed
    i32 scaling = resolveGroundAcceleration(
        self,
        world,
        3727 // ONE * 0.91
    );
    // Doesn't update position, only modifies velocity
    iPhysicsObjectMoveFlying(
        self,
        move_strafe,
        move_forward,
        scaling
    );
    // This determines sliding effect on blocks (e.g. ice vs grass)
    // correlating to how much x/z velocity will be preserved into
    // the next tick
    scaling = resolveGroundAcceleration(
        self,
        world,
        2457 //ONE * 0.6
    );
    VECTOR* velocity = &self->velocity;
    iPhysicsObjectMove(
        self,
        world,
        velocity->vx,
        velocity->vy,
        velocity->vz,
        ctx
    );
    if (!self->flags.on_ground) {
        velocity->vy -= self->config->gravity;
        velocity->vy = fixedMul(velocity->vy, 4014); // ONE * 0.98 = 4014
    }
    velocity->vx = absMinBound(fixedMul(velocity->vx, scaling), MINIMUM_VELOCITY, 0);
    velocity->vz = absMinBound(fixedMul(velocity->vz, scaling), MINIMUM_VELOCITY, 0);
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
    // NOTE: We could use the PhysicsObjectConfig::collision_intervals struct entries
    //       to allow for customisation of the detection points here instead of per-block
    //       indices. I don't think it will really help though since the AABB is already
    //       defined in terms of PhysicsObjectConfig::width & PhysicsObjectConfig::height
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
                const AABB block_aabb = (AABB) {
                    .min = vector_const_mul(position, ONE_BLOCK),
                    .max = (VECTOR) {
                        .vx = (x + 1) * ONE_BLOCK,
                        .vy = (y + 1) * ONE_BLOCK,
                        .vz = (z + 1) * ONE_BLOCK,
                    }
                };
                if (!aabbIntersects(aabb, &block_aabb)) {
                    continue;
                }
                cvector_push_back(collided_aabbs, block_aabb);
            }
        }
    }
    return collided_aabbs;
}

void updateFallState(PhysicsObject* physics_object, const i32 velocity_y, void* ctx) {
    if (physics_object->flags.on_ground) {
        if (physics_object->fall_distance > 0) {
            const PhysicsObjectFall fall_handler = physics_object->update_handlers->fall_handler;
            if (fall_handler != NULL) {
                fall_handler(physics_object, physics_object->fall_distance, ctx);
            }
            physics_object->fall_distance = 0;
        }
    } else {
        physics_object->fall_distance -= velocity_y;
    }
}

void collideWithWorld(PhysicsObject* physics_object, const World* world, i32 vel_x, i32 vel_y, i32 vel_z, void* ctx) {
    const i32 curr_vel_x = vel_x;
    const i32 curr_vel_y = vel_y;
    const i32 curr_vel_z = vel_z;
    AABB aabb = (AABB) {0};
    aabbAddCoord(&physics_object->aabb, &aabb, vel_x, vel_y, vel_z);
    const cvector(AABB) collided_aabbs = getCollidingAABBs(world, &aabb);
    AABB const* elem = NULL;
    // Determine closest y value or default to current value
    cvector_for_each_in(elem, collided_aabbs) {
        vel_y = aabbYOffset(elem, &physics_object->aabb, vel_y);
    }
    aabbOffset(&physics_object->aabb, 0, vel_y, 0);
    // Determine closest x value or default to current value
    cvector_for_each_in(elem, collided_aabbs) {
        vel_x = aabbXOffset(elem, &physics_object->aabb, vel_x);
    }
    aabbOffset(&physics_object->aabb, vel_x, 0, 0);
    // Determine closest z value or default to current value
    cvector_for_each_in(elem, collided_aabbs) {
        vel_z = aabbZOffset(elem, &physics_object->aabb, vel_z);
    }
    aabbOffset(&physics_object->aabb, 0, 0, vel_z);
    // Make the new position at the bottom centre of the AABB
    physics_object->position.vx = (physics_object->aabb.min.vx + physics_object->aabb.max.vx) >> 1;
    physics_object->position.vy = physics_object->aabb.min.vy + physics_object->config->y_offset - physics_object->y_size;
    physics_object->position.vz = (physics_object->aabb.min.vz + physics_object->aabb.max.vz) >> 1;
    physics_object->flags.collided_horizontal = curr_vel_x != vel_x || curr_vel_z != vel_z;
    physics_object->flags.collided_vertical = curr_vel_y != vel_y;
    physics_object->flags.on_ground = curr_vel_y != vel_y && curr_vel_y < 0;
    physics_object->flags.collided = physics_object->flags.collided_horizontal || physics_object->flags.collided_vertical;
    updateFallState(physics_object, vel_y, ctx);
    if (curr_vel_x != vel_x) {
        physics_object->velocity.vx = 0;
    }
    if (curr_vel_y != vel_y) {
        physics_object->velocity.vy = 0;
    }
    if (curr_vel_z != vel_z) {
        physics_object->velocity.vz = 0;
    }
    cvector_free(collided_aabbs);
}

void iPhysicsObjectMove(VSelf, World* world, const i32 velocity_x, const i32 velocity_y, const i32 velocity_z, void* ctx) __attribute__((alias("IPhysicsObject_move")));
void IPhysicsObject_move(VSelf, World* world, const i32 velocity_x, const i32 velocity_y, const i32 velocity_z, void* ctx) {
    VSELF(PhysicsObject);
    if (self->flags.no_clip) {
        aabbOffset(& self->aabb, velocity_x, velocity_y, velocity_z);
        self->position.vx = (self->aabb.min.vx + self->aabb.max.vx) >> 1;
        self->position.vy = self->aabb.min.vy + self->config->y_offset - self->y_size;
        self->position.vz = (self->aabb.min.vz + self->aabb.max.vz) >> 1;
        return;
    }
    self->y_size *= 1638; // ONE * 0.4
    collideWithWorld(
        self,
        world,
        velocity_x,
        velocity_y,
        velocity_z,
        ctx
    );
}

void iPhysicsObjectMoveFlying(VSelf, i32 move_strafe, i32 move_forward, const i32 scaling) __attribute__((alias("IPhysicsObject_moveFlying")));
void IPhysicsObject_moveFlying(VSelf, i32 move_strafe, i32 move_forward, const i32 scaling) {
    VSELF(PhysicsObject);
    // move_strafe / BLOCK_SIZE
    // => move_strafe * (1 / BLOCK_SIZE);
    // => move_strafe * 0.0142857143
    // => (move_strafe * (ONE * 0.0142857143)) >> FIXED_POINT_SHIFT
    // => (move_strafe * 58.5142857143) >> FIXED_POINT_SHIFT
    // ~> (move_strafe * 59) >> FIXED_POINT_SHIFT
    const i32 shifted_move_strafe = fixedMul(move_strafe, 59);
    const i32 shifted_move_forward = fixedMul(move_forward, 59);
    i32 dist = SquareRoot12(
        fixedMul(shifted_move_strafe, shifted_move_strafe)
        + fixedMul(shifted_move_forward, shifted_move_forward)
    );
    if (dist < 40) { // ONE * 0.01
        return;
    } else if (dist < ONE) {
        dist = ONE;
    }
    DEBUG_LOG("Scaling: %d, Dist: %d\n", scaling, dist);
    dist = (scaling << 12) / dist;
    move_strafe = fixedMul(move_strafe, dist);
    move_forward = fixedMul(move_forward, dist);
    // Adjust x/z contribution by yaw angle to make movement relative to facing direction
    const i32 sin_yaw = isin(self->rotation.yaw >> FIXED_POINT_SHIFT);
    const i32 cos_yaw = icos(self->rotation.yaw >> FIXED_POINT_SHIFT);
    self->velocity.vx += fixedMul(move_strafe, cos_yaw) - fixedMul(move_forward, sin_yaw);
    self->velocity.vz += fixedMul(move_forward, cos_yaw) + fixedMul(move_strafe, sin_yaw);
}

void iPhysicsObjectFall(VSelf, i32 distance) __attribute__((alias("IPhysicsObject_fall")));
void IPhysicsObject_fall(VSelf, i32 distance) {
    // Do nothing by default
}
