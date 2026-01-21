#include "taxicab.h"

#include <assert.h>
#include "math_utils.h"

TRad tcabAngle(const fixedi32 x, const fixedi32 y) {
    if (y >= 0) {
        return x >= 0
            ? fixedFixedDiv(y, x + y)
            : fixedFixedDiv((1 << FIXED_POINT_SHIFT) - x, -x + y); 
    }
    return x < 0
        ? fixedFixedDiv((2 << FIXED_POINT_SHIFT) - y, -x - y)
        : fixedFixedDiv((3 << FIXED_POINT_SHIFT) + x, x - y); 
}

bool tcabAngleInRange(const TRad ref,
                      const TRad angle,
                      const TRad query) {
    assert(query >= 0 && query < TRAD_MAX);
    assert(ref >= 0 && ref < TRAD_MAX);
    assert(angle >= 0 && angle < TRAD_MAX);
    const TRad a = positiveModulo(ref + angle, TRAD_MAX); 
    const TRad b = positiveModulo(ref - angle, TRAD_MAX); 
    return a < b
        ? query <= a || query >= b
        : query <= a && query >= b;
}
