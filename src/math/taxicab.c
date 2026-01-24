#include "taxicab.h"

#include <assert.h>
#include "math_utils.h"

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

bool tcabAngleInRange(const TRad ref,
                      const TRad angle,
                      const TRad query) {
    assert(ref >= 0 && ref < TRAD_MAX);
    assert(angle >= 0 && angle < TRAD_MAX);
    assert(query >= 0 && query < TRAD_MAX);
    const TRad a = positiveModulo(ref + angle, TRAD_MAX); 
    const TRad b = positiveModulo(ref - angle, TRAD_MAX); 
    return a < b
        ? query <= a || query >= b
        : query <= a && query >= b;
}
