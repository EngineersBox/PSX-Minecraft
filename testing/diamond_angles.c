#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

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
    assert(query >= 0 && query < TRAD_MAX);
    assert(ref >= 0 && ref < TRAD_MAX);
    assert(angle >= 0 && angle < TRAD_MAX);
    const TRad a = positiveModulo(ref + angle, TRAD_MAX); 
    const TRad b = positiveModulo(ref - angle, TRAD_MAX); 
    printf("Min angle: %f\n", a / 4096.0);
    printf("Max angle: %f\n", b / 4096.0);
    printf("Query: %f\n", query / 4096.0);
    return a < b
        ? query <= a || query >= b
        : query <= a && query >= b;
}

typedef int32_t fixedi32;
typedef int64_t fixedi64;

#define fixedFixedDiv(x, y) ((((fixedi64) (x)) << FIXED_POINT_SHIFT) / ((fixedi32) (y)))

#define ensureNonZero(value) ({\
    __typeof__(value) _v = (value); \
    (_v) == 0 ? 1 : (_v); \
})

TRad tcabAngle(const fixedi32 x, const fixedi32 y) {
    if (y >= 0) {
        return x >= 0
            ? fixedFixedDiv(y, ensureNonZero(x + y))
            : fixedFixedDiv((1 << FIXED_POINT_SHIFT) - x, ensureNonZero(-x + y)); 
    }
    return x < 0
        ? fixedFixedDiv((2 << FIXED_POINT_SHIFT) - y, ensureNonZero(-x - y))
        : fixedFixedDiv((3 << FIXED_POINT_SHIFT) + x, ensureNonZero(x - y)); 
}

int main() {
    // const VECTOR v0 = (VECTOR) {
    //     .vx = 0,
    //     .vy = 0,
    //     .vz = 0
    // };
    const VECTOR v = (VECTOR) {
        .vx = -(1 << FIXED_POINT_SHIFT),
        .vy = 0,
        .vz = 1 << FIXED_POINT_SHIFT,
    };
    printf("Angle XY: %f\n", diamondAngle(v.vx, v.vy) / 4096.0);
    printf("Angle XZ: %f\n", diamondAngle(v.vx, v.vz) / 4096.0);
    const TRad ref = 0;
    const TRad angle = TRAD_60_DEG >> 1;
    // const TRad query = ref;
    // const TRad query = TRAD_50_DEG;
    // const TRad query = 14198; // 340
    const TRad query = tcabAngle(v.vz, v.vz);
    printf(
        "Ref: %d Angle: %d Query: %d Result: %s\n",
        ref,
        angle,
        query,
        tcabAngleInRange(ref, angle, query) ? "true" : "false"
    );
    return 0;
}
