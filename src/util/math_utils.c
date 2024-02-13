#include "math_utils.h"

#include <stdio.h>

VECTOR rotationToDirection(const VECTOR* rotation) {
    printf("Rotation: (%d,%d,%d)\n", rotation->vx, rotation->vy, rotation->vz);
    const int32_t xz_len = icos(rotation->vx >> FIXED_POINT_SHIFT) << FIXED_POINT_SHIFT;
    return (VECTOR) {
        .vx = xz_len * icos(rotation->vy >> FIXED_POINT_SHIFT),
        .vy = isin(rotation->vx >> FIXED_POINT_SHIFT) << FIXED_POINT_SHIFT,
        .vz = xz_len * isin(-rotation->vy >> FIXED_POINT_SHIFT)
    };
}