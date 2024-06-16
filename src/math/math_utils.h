#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>
#include <psxgte.h>

#include "../util/inttypes.h"
#include "fixed_point.h"

/**
 * @brief Compute the maximum of two numbers
 * @param a - first number
 * @param b - second number
 * @return a if a > b, otherwise b
 */
#define max(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b; \
})

/**
 * @brief Compute the minimum of two numbers
 * @param a - first number
 * @param b - second number
 * @return a if a < b, otherwise b
 */
#define min(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b; \
})

#define absMinBound(value, bound, minimum) ({ \
    __typeof__(value) _value = (value); \
    (absv(_value) <= (bound) ? (minimum) : _value); \
})

#define quadIntersect(cursor, base, dim) ( \
    (cursor)->vx >= (base)->vx \
    && (cursor)->vy >= (base)->vy \
    && (cursor)->vx <= (base)->vx + (dim)->vx \
    && (cursor)->vy <= (base)->vy + (dim)->vy \
)

/**
 * @brief Compute the second power of v as v^2 or v * v
 * @param v - Value to square
 * @return v * v
 */
#define pow2(v) ({ \
    __typeof__(v) _v = (v); \
    _v * _v; \
})

#define ceilDiv(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (i32)((_a + _b - 1) / _b); \
})

#define floorDiv(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    const i32 d = _a / _b; \
    const i32 r = _a % _b; \
    (r ? (d - ((_a < 0) ^ (_b < 0))) : d); \
})

#define squareDistance(v1, v2) (pow2((v2)->vx - (v1)->vx) + pow2((v2)->vy - (v1)->vy) + pow2((v2)->vz - (v1)->vz))

/**
 * @brief Clamp a value between an upper and lower bound
 * @param x - value
 * @param lower - lower bound on value
 * @param upper - upper bound on value
 * @return x if lower <= x <= upper, lower if x < lower, higher if x > higher
 */
#define clamp(x, lower, upper) (min(upper, max(x, lower)))

/**
 * @brief Compare two numbers and return an integer indicating larger value
 * @param a - first number
 * @param b - second number
 * @return 0 if a == b, 1 if b > a, -1 if b < a
 */
#define cmp(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_b > _a) - (_b < _a); \
})

/**
 * @brief Retrieve the sign (1 for positive, 0 for negative) of a number
 * @param v - number to apply to
 * @return 1 if v >= 0, otherwise 0
 */
#define sign(v) cmp(0, v)

/**
 * @brief Absolute value of an number
 * @param v - number to apply to
 * @return -v if v < 0, otherwise v
 */
#define absv(v) ({ \
    __typeof__(v) _v = (v); \
    _v * sign(_v); \
})

#define positiveModulo(i, n) ({ \
    __typeof__(n) _n = (n); \
    i32 m = (i) % _n; \
    m + ((m >> 31) & _n); \
})

#define isPowerOf2(x) (((x) & ((x) - 1)) == 0)

// Convert degrees to unit range accepted by trig functions
// E.g. degToUnitRange(45) = 512
#define degToUnitRange(deg) (((deg) << 12) / 360)

#endif //MATH_UTILS_H
