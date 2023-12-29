#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>

#define max(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b; \
})
#define min(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b; \
})
#define clamp(x, lower, upper) (min((upper), max((x), (lower))))

static void crossProduct(const SVECTOR* v0, const SVECTOR* v1, VECTOR* out) {
    out->vx = ((v0->vy * v1->vz) - (v0->vz * v1->vy)) >> 12;
    out->vy = ((v0->vz * v1->vx) - (v0->vx * v1->vz)) >> 12;
    out->vz = ((v0->vx * v1->vy) - (v0->vy * v1->vx)) >> 12;
}

typedef struct _BVECTOR {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t pad;
} BVECTOR;

#endif //MATH_UTILS_H
