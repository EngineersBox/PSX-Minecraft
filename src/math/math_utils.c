#include "math_utils.h"

#include <logging.h>
#include <stdio.h>

VECTOR rotationToDirection(const VECTOR* rotation) {
    // printf("Rotation: (%d,%d,%d)\n", rotation->vx, rotation->vy, rotation->vz);
    const int32_t x = rotation->vx >> FIXED_POINT_SHIFT;
    const int32_t y = rotation->vy >> FIXED_POINT_SHIFT;
    const int32_t xz_len = icos(x);
    return (VECTOR) {
        .vx = (xz_len * isin(-y)) >> FIXED_POINT_SHIFT,
        .vy = -isin(x), // Negation to conver from -Y up to +Y up coordinate space
        .vz = (xz_len * icos(y)) >> FIXED_POINT_SHIFT
    };
}

void _crossProduct(const SVECTOR *v0, const SVECTOR *v1, VECTOR *out) {
    out->vx = ((v0->vy * v1->vz) - (v0->vz * v1->vy)) >> FIXED_POINT_SHIFT;
    out->vy = ((v0->vz * v1->vx) - (v0->vx * v1->vz)) >> FIXED_POINT_SHIFT;
    out->vz = ((v0->vx * v1->vy) - (v0->vy * v1->vx)) >> FIXED_POINT_SHIFT;
}

INLINE VECTOR cross(const VECTOR* v0, const VECTOR* v1) {
    return vec3_i32(
        ((v0->vy * v1->vz) - (v0->vz * v1->vy)) >> FIXED_POINT_SHIFT,
        ((v0->vz * v1->vx) - (v0->vx * v1->vz)) >> FIXED_POINT_SHIFT,
        ((v0->vx * v1->vy) - (v0->vy * v1->vx)) >> FIXED_POINT_SHIFT
    );
}

INLINE VECTOR _cross(const VECTOR v0, const VECTOR v1) {
    return vec3_i32(
        (fixedMul(v0.vy, v1.vz) - fixedMul(v0.vz, v1.vy)),
        (fixedMul(v0.vz, v1.vx) - fixedMul(v0.vx, v1.vz)),
        (fixedMul(v0.vx, v1.vy) - fixedMul(v0.vy, v1.vx))
    );
}

INLINE i32 dot(const VECTOR* v0, const VECTOR* v1) {
    return fixedMul(v0->vx, v1->vx)
        + fixedMul(v0->vy, v1->vy)
        + fixedMul(v0->vz, v1->vz);
}

i64 _dot(const VECTOR v0, const VECTOR v1) {
    const i64 v0x = v0.vx;
    const i64 v0y = v0.vy;
    const i64 v0z = v0.vz;
    const i64 v1x = v1.vx;
    const i64 v1y = v1.vy;
    const i64 v1z = v1.vz;
    return fixedMul(v0x, v1x)
        + fixedMul(v0y, v1y)
        + fixedMul(v0z, v1z);
}

VECTOR vec3_i32_normalize(const VECTOR v) {
    const fixedi32 length = SquareRoot12(_dot(v,v));
    DEBUG_LOG("[MATH] Normalize. Source: (%d,%d,%d), Length: %d\n", inlineVec(v), length);
    return vec3_i32(
        (v.vx << FIXED_POINT_SHIFT) / length,
        (v.vy << FIXED_POINT_SHIFT) / length,
        (v.vz << FIXED_POINT_SHIFT) / length
    );
}

// 4096 / ((((a & 0xffff) << 12) / (b & 0xffff)) >> 12)
// ((a & 0xffff) * (b & 0xffff)) >> 12

// 0.7 * 0.2 = 0.14