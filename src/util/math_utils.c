#include "math_utils.h"

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

// 4096 / ((((a & 0xffff) << 12) / (b & 0xffff)) >> 12)
// ((a & 0xffff) * (b & 0xffff)) >> 12

// 0.7 * 0.2 = 0.14