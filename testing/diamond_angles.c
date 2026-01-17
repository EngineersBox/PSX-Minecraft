#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;
typedef int64_t i64;

typedef struct {
    i32 vx;
    i32 vy;
    i32 vz;
} VECTOR;

#define FIXED_POINT_SHIFT 12
#define fixedMul(x, y) (((x) * (y)) >> FIXED_POINT_SHIFT)
// #define fixedDiv(x, y) ((((x) << FIXED_POINT_SHIFT) / (y)) >> FIXED_POINT_SHIFT)
#define fixedDiv(x, y) ((((i64) x) << FIXED_POINT_SHIFT) / ((i32) y))

i32 dot(const VECTOR* v0, const VECTOR* v1) {
    return fixedMul(v0->vx, v1->vx)
            + fixedMul(v0->vy, v1->vy)
            + fixedMul(v0->vz, v1->vz);
}

VECTOR cross(const VECTOR* v0, const VECTOR* v1) {
    return (VECTOR) {
        (fixedMul(v0->vy, v1->vz) - fixedMul(v0->vz, v1->vy)), \
        (fixedMul(v0->vz, v1->vx) - fixedMul(v0->vx, v1->vz)), \
        (fixedMul(v0->vx, v1->vy) - fixedMul(v0->vy, v1->vx)) \
    };
}

i32 len(const VECTOR v) {
    return (v.vx * v.vx) + (v.vy * v.vy) + (v.vz * v.vz);
}

// Assumes comparison to vector of (1,0)
// Source: https://stackoverflow.com/a/14675998/11001270
// Source: https://www.freesteel.co.uk/wpblog/2009/06/05/encoding-2d-angles-without-trigonometry/
i32 diamondAngle(const i32 x, const i32 y) {
    if (y >= 0)
        return (x >= 0 ? fixedDiv(y, (x+y)) : fixedDiv((1 << FIXED_POINT_SHIFT)-x, (-x+y))); 
    else
        return (x < 0 ? fixedDiv((2 << FIXED_POINT_SHIFT)-y, (-x-y)) : fixedDiv((3 << FIXED_POINT_SHIFT)+x, (x-y))); 
}

int main() {
    // const VECTOR v0 = (VECTOR) {
    //     .vx = 0,
    //     .vy = 0,
    //     .vz = 0
    // };
    const VECTOR v = (VECTOR) {
        .vx = 1 << FIXED_POINT_SHIFT,
        .vy = -(3 << FIXED_POINT_SHIFT),
        .vz = 2 << FIXED_POINT_SHIFT,
    };
    // NOTE: If camera is treated as if pointing towards (1,0,0), then
    // we can transform the query point (chunk pos) to be relative to that.
    // Then we can compute the diamondAngle for X,Y and X,Z for the two
    // angles that define the point relative to the camera. If both of
    // these exceed the FOV angle (in t-rads) for positive and negative
    // sides, then the chunk is within the frustum. The question is how
    // to transform the chunk location to be relative to (1,0,0) facing
    // camera.
    printf("Angle XY: %f\n", diamondAngle(v.vx, v.vy) / 4096.0);
    printf("Angle XZ: %f\n", diamondAngle(v.vx, v.vz) / 4096.0);
    return 0;
}
