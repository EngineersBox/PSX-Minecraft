#include "math_utils.h"

VECTOR rotationToDirection(const VECTOR* rotation) {
    int32_t xz_len = icos(rotation.vx);
    return (VECTOR) {
        .vx = xz_len * icos(rotation.vy),
        .vy = isin(rotation.vx),
        .vz = xz_len * isin(-rotation.vy)
    };
}