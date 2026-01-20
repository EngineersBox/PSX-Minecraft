#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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

/**
 * Q12 fixed point taxicab radians
 * Range: 0 - 16384 (4 << FIXED_POINT_SHIFT)
 */
typedef int32_t TRad;

#define TRAD_MIN 0
#define TRAD_MAX (4 << FIXED_POINT_SHIFT)

// (2-(2/(1+tan(45deg)))) * 4096 = 4096
#define TRAD_45_DEG 4096
// (2-(2/(1+tan(50deg)))) * 4096 = 4454.35356...
#define TRAD_50_DEG 4454
// (2-(2/(1+tan(60deg)))) * 4096 = 5193.51989...
#define TRAD_60_DEG 5194
// (2-(2/(1+tan(70deg)))) * 4096 = 6005.99616...
#define TRAD_70_DEG 6006
// (2-(2/(1+tan(80deg)))) * 4096 = 6964.05007
#define TRAD_80_DEG 6964
// Note: tan(90) is undefined, so we take the limit
// lim x->90 (2-(2/(1+tan(x deg)))) * 4096 = 8192
#define TRAD_90_DEG 8192

#define positiveModulo(i, n) ({ \
    __typeof__(n) _n = (n); \
    i32 m = (i) % _n; \
    m + ((m >> 31) & _n); \
})

bool tcabAngleInRange(const TRad ref,
                      const TRad angle,
                      const TRad query) {
    TRad a = positiveModulo(ref - angle, TRAD_MAX); 
    TRad b = positiveModulo(ref + angle, TRAD_MAX); 
    printf("Min angle: %d\n", a);
    printf("Max angle: %d\n", b);
    const bool result = query >= a && query <= b;
    return a < b ? result : !result;
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
    //
    // We can resolve this issue simply by using diamongAngle(...) with
    // the camera angle initially, then checking if the chunk angle is
    // within some angle difference (i.e. within +-60deg as t-rads).
    printf("Angle XY: %f\n", diamondAngle(v.vx, v.vy) / 4096.0);
    printf("Angle XZ: %f\n", diamondAngle(v.vx, v.vz) / 4096.0);
    const TRad ref = 4688; // 20 deg
    const TRad angle = TRAD_60_DEG;
    // const TRad query = TRAD_50_DEG;
    // const TRad query = 14198; // 340
    const TRad query = 9420; // 280
    printf(
        "Ref: %d Angle: %d Query: %d Result: %s\n",
        ref,
        angle,
        query,
        tcabAngleInRange(ref, angle, query) ? "true" : "false"
    );
    return 0;
}
