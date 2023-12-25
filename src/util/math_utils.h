#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

static void crossProduct(const SVECTOR* v0, const SVECTOR* v1, VECTOR* out) {
    out->vx = ((v0->vy * v1->vz) - (v0->vz * v1->vy)) >> 12;
    out->vy = ((v0->vz * v1->vx) - (v0->vx * v1->vz)) >> 12;
    out->vz = ((v0->vx * v1->vy) - (v0->vy * v1->vx)) >> 12;
}

#endif //MATH_UTILS_H
