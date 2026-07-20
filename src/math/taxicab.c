#include "taxicab.h"

#include <assert.h>
#include <psxgte.h>

#define ensureNonZero(value) ({\
    __typeof__(value) _v = (value); \
    (_v) == 0 ? 1 : (_v); \
})

TRad tcabAngle(const fixedi32 x, const fixedi32 y) {
    if (y >= 0) {
        return (x >= 0
            ? fixedFixedDiv(y, ensureNonZero(x + y))
            : fixedFixedDiv(ONE - x, ensureNonZero(-x + y))) % TRAD_MAX; 
    }
    return (x < 0
        ? fixedFixedDiv((2 << FIXED_POINT_SHIFT) - y, ensureNonZero(-x - y))
        : fixedFixedDiv((3 << FIXED_POINT_SHIFT) + x, ensureNonZero(x - y))) % TRAD_MAX;
}

bool tcabAngleInRange(TRad ref,
                      const TRad angle,
                      TRad query) {
    ref = ensureNonZero(ref);
    assert(ref >= 0 && ref < TRAD_MAX);
    assert(angle >= 0 && angle < TRAD_MAX);
    assert(query >= 0 && query < TRAD_MAX);
    const TRad a = (ref + TRAD_MAX) + angle;
    const TRad b = (ref + TRAD_MAX) - angle;
    query += TRAD_MAX;
    return query >= a && query <= b;
}

bool tcabAngleRangeOverlap(TRad ref_a,
                           const TRad angle_a,
                           TRad ref_b,
                           const TRad angle_b) {
    ref_a = ensureNonZero(ref_a);
    ref_b = ensureNonZero(ref_b);
    assert(ref_a >= 0 && ref_a < TRAD_MAX);
    assert(ref_b >= 0 && ref_b < TRAD_MAX);
    assert(angle_a >= 0 && angle_a < TRAD_MAX);
    assert(angle_b >= 0 && angle_b < TRAD_MAX);
    const TRad a_start = (ref_a + TRAD_MAX) - angle_a;
    const TRad a_end = (ref_a + TRAD_MAX) + angle_a;
    const TRad b_start = (ref_b + TRAD_MAX) - angle_b;
    const TRad b_end = (ref_b + TRAD_MAX) + angle_b;
    return a_start <= b_end && b_start <= a_end;
}
