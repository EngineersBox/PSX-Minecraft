#pragma once

#ifndef _PSXMC__MATH__FIXED_POINT_H_
#define _PSXMC__MATH__FIXED_POINT_H_

#include "../util/inttypes.h"

/**
 * @brief Factor to shift left/right to convert int to/from fixed-point format
 */
#define FIXED_POINT_SHIFT 12
#define TRIG_5TH_ORDER_REDUCTION 9
#define FIXED_PI 16384
/**
 * Implements the 5th-order polynomial approximation to sin(x).
 *
 * Sourced from https://www.nullhardware.com/blog/fixed-point-sine-and-cosine-for-embedded-systems/
 *
 * @param i   angle (with 2^15 units/circle: [0,32768])
 * @return    16 bit fixed point Sine value (4.12) (ie: +4096 = +1 & -4096 = -1)
 * @note The result is accurate to within +- 1 count. ie: +/-2.44e-4.
 */
i32 sin5o(i16 i);
i32 cos5o(i16 i);

// Fixed point

#define FIXED_MASK_FRACTIONAL 0xFFF
#define FIXED_MASK_WHOLE (~FIXED_MASK_FRACTIONAL)
#define FIXED_POINT_MAX FIXED_MASK_WHOLE

#define fixedGetFractional(value) ((value) & FIXED_MASK_FRACTIONAL)
#define fixedGetWhole(value) ((value) >> FIXED_POINT_SHIFT)

/**
 * @brief Divides two fixed point int16_t numbers as x / y
 * @param x Dividend (number being divided)
 * @param y Divisor (number performing division)
 * @return Result of division x / y
 */
#define fixedDiv(x, y) ((((x) << FIXED_POINT_SHIFT) / (y)) >> FIXED_POINT_SHIFT)

/**
 * @brief Divides a fixed point number by an integer as x / y
 * @param x Dividend (fixed-point number being divided)
 * @param y Divisor (integer number performing division)
 * @return Result of division x / y
 */
#define fixedIntDiv(x, y) (((fixedi32) x) / ((i32) y))

/**
 * @brief Divides two fixed point numbers as x / y
 * @param x Dividend (number being divided)
 * @param y Divisor (number performing division)
 * @return Result of division x / y
 */
#define fixedFixedDiv(x, y) ((((fixedi64) (x)) << FIXED_POINT_SHIFT) / ((fixedi32) (y)))

/**
 * @brief Multiply two fixed point numbers as x_w.x_f * y_w.w_f:
 *        For example 0.7 * 0.2 = 0.14 <=> (2867 * 819) >> FIXED_POINT_SHIFT = 573
 * @param x fractional number
 * @param y fractional number
 * @return Result of multiplication
 */
#define fixedMul(x, y) ((fixedi32)(((fixedi64)(x) * (fixedi64)(y)) >> FIXED_POINT_SHIFT))

#define FIXED_1_2 (fixedDiv(ONE, 2))
#define FIXED_1_4 (fixedDiv(ONE, 4))
#define FIXED_1_8 (fixedDiv(ONE, 8))
#define FIXED_1_16 (fixedDiv(ONE, 16))
#define FIXED_1_32 (fixedDiv(ONE, 32))
#define FIXED_1_64 (fixedDiv(ONE, 64))
#define FIXED_1_128 (fixedDiv(ONE, 128))
#define FIXED_1_256 (fixedDiv(ONE, 256))
#define FIXED_1_512 (fixedDiv(ONE, 512))
#define FIXED_1_1024 (fixedDiv(ONE, 1024))
#define FIXED_1_2048 (fixedDiv(ONE, 2048))
#define FIXED_1_4096 (fixedDiv(ONE, ONE))

#define fixedFloor(v, factor) ({ \
    __typeof__(v) _v = (v);\
    _v - positiveModulo(_v, (factor)); \
})

#endif // _PSXMC__MATH__FIXED_POINT_H_
