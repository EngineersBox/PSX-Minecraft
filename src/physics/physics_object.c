#include "physics_object.h"

#include "../math/math_utils.h"

void iPhysicsObjectUpdate(VSelf) __attribute__((alias("IPhysicsObject_update")));
void IPhysicsObject_update(VSelf) {

}

void iPhysicsObjectMove(VSelf, u32 x, u32 y, u32 z) __attribute__((alias("IPhysicsObject_move")));
void IPhysicsObject_move(VSelf, u32 x, u32 y, u32 z) {
    VSELF(PhysicsObject);
    if (self->flags.no_clip) {
        aabbOffset(&self->aabb, x, y, z);
        self->position.vx = (self->aabb.min_x + self->aabb.max_x) >> 1;
        self->position.vy = self->aabb.min_y + self->y_offset - self->y_size;
        self->position.vz = (self->aabb.min_z + self->aabb.max_z) >> 1;
        return;
    }
    
}

void iPhysicsObjectMoveWithHeading(VSelf) __attribute__((alias("IPhysicsObject_moveWithHeading")));
void IPhysicsObject_moveWithHeading(VSelf) {

}
