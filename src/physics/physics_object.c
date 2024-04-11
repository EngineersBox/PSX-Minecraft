#include "physics_object.h"

#include "../math/math_utils.h"
#include "../util/interface99_extensions.h"
#include "../game/blocks/block.h"

/* Order:
 * update(...) { ... moveWithHeading(...) ... }
 * moveWithHeading(...) { ... move(...) ... }
 * move(...) { ... }
 */

void iPhysicsObjectUpdate(VSelf, World* world) __attribute__((alias("IPhysicsObject_update")));
void IPhysicsObject_update(VSelf, World* world) {

}

void iPhysicsObjectMoveWithHeading(VSelf, World* world) __attribute__((alias("IPhysicsObject_moveWithHeading")));
void IPhysicsObject_moveWithHeading(VSelf, World* world) {
    VSELF(PhysicsObject);
    iPhysicsObjectMove(self, world, self->motion.vx, self->motion.vy, self->motion.vz);
}

#define ONE_BLOCK (BLOCK_SIZE << FIXED_POINT_SHIFT)

bool collideWithWorld(PhysicsObject* physics_object, World* world, i32 motion_x, i32 motion_y, i32 motion_z) {
    AABB* aabb = &physics_object->aabb;
    // https://github.com/camthesaxman/cubecraft/blob/1dd4f9f25069bfaba0ac659c845c5eccbea4c08a/source/field.c#L318
    bool test_x = false;
    bool test_y = false;
    bool test_z = false;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    bool collision_detected = false;
#define applyMotion(v) \
    if (motion_##v < 0) { \
        v = aabb->min_##v + motion_##v; \
        test_##v = true; \
    } else if (motion_##v > 0) { \
        v = aabb->max_##v + motion_##v; \
        test_##v = true; \
    }
    applyMotion(x);
    applyMotion(y);
    applyMotion(z);
    VECTOR new_position = {};
    if (test_x) {
        bool x_collision = false;
        for (i32 aabb_y = aabb->min_y; aabb_y <= aabb->max_y && !x_collision; aabb_y += ONE) {
            for (i32 aabb_z = aabb->min_z; aabb_z <= aabb->max_z && !x_collision; aabb_z += ONE) {
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
            collision_detected = true;
        } else {
            new_position.vx = motion_x;
        }
    }
    if (test_z) {
        bool z_collision = false;
        for (i32 aabb_y = aabb->min_y; aabb_y <= aabb->max_y && !z_collision; aabb_y += ONE) {
            for (i32 aabb_x = aabb->min_x; aabb_x <= aabb->max_x && !z_collision; aabb_x += ONE) {
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
            collision_detected = true;
        } else {
            new_position.vz = motion_z;
        }
    }
    if (test_y) {
        bool y_collision = false;
        for (i32 aabb_x = aabb->min_x; aabb_x <= aabb->max_x && !y_collision; aabb_x += ONE) {
            for (i32 aabb_z = aabb->min_z; aabb_z <= aabb->max_z && !y_collision; aabb_z += ONE) {
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
                new_position.vy = ONE;
                physics_object->flags.on_ground = true;
            }
            physics_object->velocity.vy = 0;
            collision_detected = true;
        } else {
            new_position.vy = motion_y;
        }
    }
    aabbOffset(aabb, new_position.vx, new_position.vy, new_position.vz);
    physics_object->position = vector_add(
        physics_object->position,
        new_position
    );
//     if (physics_object->flags.on_ground) {
//         const VECTOR below_position = (VECTOR) {
//             .vx = physics_object->position.vx / ONE_BLOCK,
//             .vy = (physics_object->position.vy - ONE) / ONE_BLOCK,
//             .vz = physics_object->position.vz / ONE_BLOCK
//         };
//         const IBlock* iblock = worldGetBlock(world, &below_position);
//         if (iblock == NULL) {
//             goto return_collision_state;
//         }
//         const Block* block = VCAST_PTR(Block*, iblock);
//         if (block->type != BLOCKTYPE_EMPTY) {
//             physics_object->flags.on_ground = true;
//         }
//     }
// return_collision_state:
    return collision_detected;
}

void iPhysicsObjectMove(VSelf, World* world, i32 x, i32 y, i32 z) __attribute__((alias("IPhysicsObject_move")));
void IPhysicsObject_move(VSelf, World* world, i32 x, i32 y, i32 z) {
    VSELF(PhysicsObject);
    if (self->flags.no_clip) {
        aabbOffset(&self->aabb, x, y, z);
        self->position.vx = (self->aabb.min_x + self->aabb.max_x) >> 1;
        self->position.vy = self->aabb.min_y + self->y_offset - self->y_size;
        self->position.vz = (self->aabb.min_z + self->aabb.max_z) >> 1;
        return;
    }
    collideWithWorld(
        self,
        world,
        self->motion.vx,
        self->motion.vy,
        self->motion.vz
    );
}
