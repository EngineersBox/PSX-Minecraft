#include "taxicab.h"

#include <assert.h>
#include <psxgte.h>
#include "math_utils.h"
#include "../logging/logging.h"

#define ensureNonZero(value) ({\
    __typeof__(value) _v = (value); \
    (_v) == 0 ? 1 : (_v); \
})

TRad _tcabAngle(const fixedi32 x, const fixedi32 y) {
    if (y >= 0) {
        return x >= 0
            ? fixedFixedDiv(y, ensureNonZero(x + y))
            : fixedFixedDiv(ONE - x, ensureNonZero(-x + y));
    }
    return x < 0
        ? fixedFixedDiv((2 << FIXED_POINT_SHIFT) - y, ensureNonZero(-x - y))
        : fixedFixedDiv((3 << FIXED_POINT_SHIFT) + x, ensureNonZero(x - y));
}

INLINE TRad tcabAngle(const fixedi32 x, const fixedi32 y) {
    return clamp(_tcabAngle(x, y), 0, TRAD_MAX);
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

bool tcabAngleRangeOverlap(TRad ref,
                           const TRad angle,
                           TRad range_start,
                           TRad range_end) {
    assert(ref >= 0 && ref < TRAD_MAX);
    assert(angle >= 0 && angle < TRAD_MAX);
    assert(range_start >= 0 && range_start < TRAD_MAX);
    assert(range_end >= 0 && range_end < TRAD_MAX);
    const TRad a_start = (ref + TRAD_MAX) - angle;
    const TRad a_end = (ref + TRAD_MAX) + angle;
    range_start += TRAD_MAX;
    range_end += TRAD_MAX;
    DEBUG_LOG("Ref range: [%d,%d] Query range: [%d,%d]\n", a_start, a_end, range_start, range_end);
    return a_start <= range_end && range_start <= a_end;
}
