#include "physics_object.h"

void physicsUpdate(PhysicsObject* physobj) {
    physobj->position.vx += physobj->direction.vx * physobj->momentum.vx;
    physobj->position.vy += physobj->direction.vy * physobj->momentum.vy;
    physobj->position.vz += physobj->direction.vz * physobj->momentum.vz;
}