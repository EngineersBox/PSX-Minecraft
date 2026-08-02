#include "taxicab.h"

#include <assert.h>
#include <psxgte.h>
#include "fixed_point.h"
#include "math_utils.h"
#include "../logging/logging.h"

#define ensureNonZero(value) ({\
    __typeof__(value) _v = (value); \
    (_v) == 0 ? 1 : (_v); \
})

TRad _tcabAngle2(const fixedi32 x, const fixedi32 y) {
    if (y >= 0) {
        return x >= 0
            ? fixedFixedDiv(y, ensureNonZero(x + y))
            : fixedFixedDiv(ONE - x, ensureNonZero(-x + y));
    }
    return x < 0
        ? fixedFixedDiv((2 << FIXED_POINT_SHIFT) - y, ensureNonZero(-x - y))
        : fixedFixedDiv((3 << FIXED_POINT_SHIFT) + x, ensureNonZero(x - y));
}

TRad _tcabAngle(const fixedi32 x, const fixedi32 y) {
    if (y >= 0) {
        return x >= 0
            ? fixedFixedDiv(x, ensureNonZero(x + y)) + (3 << FIXED_POINT_SHIFT)
            : fixedFixedDiv(y, ensureNonZero(-x + y)) + (2 << FIXED_POINT_SHIFT);
    }
    return x >= 0
        ? fixedFixedDiv(-y, ensureNonZero(x + -y))
        : fixedFixedDiv(-x, ensureNonZero(-x + -y)) + ONE;
}

INLINE TRad tcabAngle(const fixedi32 x, const fixedi32 y) {
    return clamp(_tcabAngle(x, y), 0, TRAD_MAX - 1);
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

#define isRangeSplit(range_start, range_end) ((range_end) <= (range_start))

#define rangeOverlap(ref_start, ref_end, query_start, query_end) \
    ((query_start) <= (ref_end) && (ref_start) <= query_end)

bool tcabAngleRangeOverlap(TRad ref,
                           const TRad angle,
                           TRad range_start,
                           TRad range_end) {
    assert(ref >= 0 && ref < TRAD_MAX);
    assert(angle >= 0 && angle < TRAD_MAX);
    assert(range_start >= 0 && range_start < TRAD_MAX);
    assert(range_end >= 0 && range_end < TRAD_MAX);
    const TRad ref_start = positiveModulo(ref - angle, TRAD_MAX);
    const TRad ref_end = positiveModulo(ref + angle, TRAD_MAX);
    const bool ref_split = isRangeSplit(ref_start, ref_end);
    const bool query_split = isRangeSplit(range_start, range_end);
    if (ref_split && query_split) {
        const TRad query_left_start = 0;
        const TRad query_left_end = range_end;
        const TRad query_right_start = range_start;
        const TRad query_right_end = TRAD_MAX - 1;
        const TRad ref_left_start = 0;
        const TRad ref_left_end = ref_end;
        const TRad ref_right_start = ref_start;
        const TRad ref_right_end = TRAD_MAX - 1;
        return rangeOverlap(ref_left_start, ref_left_end, query_left_start, query_left_end)
            || rangeOverlap(ref_left_start, ref_left_end, query_right_start, query_right_end)
            || rangeOverlap(ref_right_start, ref_right_end, query_left_start, query_left_end)
            || rangeOverlap(ref_right_start, ref_right_end, query_right_start, query_right_end);
    } else if (!ref_split && query_split) {
        const TRad query_left_start = 0;
        const TRad query_left_end = range_end;
        const TRad query_right_start = range_start;
        const TRad query_right_end = TRAD_MAX - 1;
        return rangeOverlap(ref_start, ref_end, query_left_start, query_left_end)
            || rangeOverlap(ref_start, ref_end, query_right_start, query_right_end);
    } else if (ref_split && !query_split) {
        const TRad ref_left_start = 0;
        const TRad ref_left_end = ref_end;
        const TRad ref_right_start = ref_start;
        const TRad ref_right_end = TRAD_MAX - 1;
        return rangeOverlap(ref_left_start, ref_left_end, range_start, range_end)
            || rangeOverlap(ref_right_start, ref_right_end, range_start, range_end);
    }
    return rangeOverlap(ref_start, ref_end, range_start, range_end);
}
