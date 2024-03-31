#include "physics_object.h"

#include "../math/math_utils.h"

VECTOR force(uint16_t ticks_passed, VECTOR* position) {

}

void physicsUpdate(PhysicsObject* physobj) {
    const VECTOR acceleration = vector_const_div(force(physobj->ticks_passed, &physobj->position), physobj->mass);
    physobj->ticks_passed += 1;
    physobj->position.vx += physobj->ticks_passed * (physobj->velocity.vx + (physobj->ticks_passed * (acceleration.vx  / 2)));
    physobj->position.vy += physobj->ticks_passed * (physobj->velocity.vy + (physobj->ticks_passed * (acceleration.vy  / 2)));
    physobj->position.vz += physobj->ticks_passed * (physobj->velocity.vz + (physobj->ticks_passed * (acceleration.vz  / 2)));
    physobj->position = vector_add(
        physobj->position,
        vector_const_mul(
            vector_add(
                physobj->velocity,
                vector_const_mul(
                    vector_const_div(acceleration, 2),
                    physobj->ticks_passed
                )
            ),
            physobj->ticks_passed
        )
    );
}