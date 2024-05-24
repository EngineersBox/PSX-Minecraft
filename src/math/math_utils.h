#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>
#include <psxgte.h>

#include "../util/preprocessor.h"
#include "../util/inttypes.h"

/**
 * @brief Factor to shift left/right to convert int to/from fixed-point format
 */
#define FIXED_POINT_SHIFT 12

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
 * @brief Computer the second power of v as v^2 or v * v
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

typedef struct _BVECTOR {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t pad;
} BVECTOR;

typedef struct _LVECTOR {
    i64 vx;
    i64 vy;
    i64 vz;
} LVECTOR;

#define _cross(type, v0, v1) ({ \
    __typeof__(v0) _v0 = (v0); \
    __typeof__(v1) _v1 = (v1); \
    ((type) { _vec3_layout( \
        (fixedMul(_v0.vy, _v1.vz) - fixedMul(_v0.vz, _v1.vy)), \
        (fixedMul(_v0.vz, _v1.vx) - fixedMul(_v0.vx, _v1.vz)), \
        (fixedMul(_v0.vx, _v1.vy) - fixedMul(_v0.vy, _v1.vx)) \
    )}); \
})
#define cross_i16(v0, v1) _cross(SVECTOR, v0, v1)
#define cross_i32(v0, v1) _cross(VECTOR, v0, v1)
#define cross_i64(v0, v1) _cross(LVECTOR, v0, v1)

#define _dot(type, v0, v1) (fixedMul((type)(v0).vx, (type)(v1).vx) \
    + fixedMul((type)(v0).vy, (type)(v1).vy) \
    + fixedMul((type)(v0).vz, (type)(v1).vz))
#define dot_i16(v0, v1) _dot(i32, v0, v1)
#define dot_i32(v0, v1) _dot(i32, v0, v1)
#define dot_i64(v0, v1) _dot(i64, v0, v1)

// Fixed point

#define FIXED_MASK_FRACTIONAL 0xFFF
#define FIXED_MASK_WHOLE (~FIXED_MASK_FRACTIONAL)
#define FIXED_POINT_MAX FIXED_MASK_WHOLE

#define fixedGetFractional(value) ((value) & FIXED_MASK_FRACTIONAL)
#define fixedGetWhole(value) ((value) >> FIXED_POINT_SHIFT)

/**
 * @brief Divides two fixed point int16_t numbers as x / y
 * @param x dividend (number being divided)
 * @param y divisor (number performing division)
 * @return Result of division x / y
 */
#define fixedDiv(x, y) ((((x) << FIXED_POINT_SHIFT) / (y)) >> FIXED_POINT_SHIFT)

/**
 * @brief Multiply two fixed point numbers as x_w.x_f * y_w.w_f:
 *        For example 0.7 * 0.2 = 0.14 <=> (2867 * 819) >> FIXED_POINT_SHIFT = 573
 * @param x fractional number
 * @param y fractional number
 * @return Result of multiplication
 */
#define fixedMul(x, y) (((x) * (y)) >> FIXED_POINT_SHIFT)

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

// Vector init

#define _vec2_layout(x, y) .vx = (x), .vy = (y)
#define _vec3_layout(x, y, z) _vec2_layout(x, y), .vz = (z)
#define vec3_i32(x, y, z) ((VECTOR) { _vec3_layout(x, y, z) })
#define vec3_i16(x, y, z) ((SVECTOR) { _vec3_layout(x, y, z) })
#define vec3_i8(x, y, z) ((CVECTOR) { _vec3_layout(x, y, z) })
#define vec2_i16(x, y) ((DVECTOR) { _vec2_layout(x, y) })

// Unified vector init

#define vec3_i32_all(v) vec3_i32(v, v, v)
#define vec3_i16_all(v) vec3_i16(v, v, v)
#define vec3_i8_all(v) vec3_i8(v, v, v)
#define vec2_i16_all(v) vec2_i16(v, v)

// Swizzle

#define _vec2_layout_swizzle(_v, x, y) _vec2_layout( \
    (_v).GLUE(v,x), \
    (_v).GLUE(v,y), \
)
#define _vec3_layout_swizzle(_v, x, y, z) _vec3_layout( \
    (_v).GLUE(v,x), \
    (_v).GLUE(v,y), \
    (_v).GLUE(v,z), \
)
#define vec3_i32_swizzle(_v, x, y, z) ((VECTOR) { _vec3_layout_swizzle(_v, x, y, z) })
#define vec3_i16_swizzle(_v, x, y, z) ((SVECTOR) { _vec3_layout_swizzle(_v, x, y, z) })
#define vec3_i8_swizzle(_v, x, y, z) ((CVECTOR) { _vec3_layout_swizzle(_v, x, y, z) })
#define vec2_i16_swizzle(_v, x, y) ((DVECTOR) { _vec2_layout_swizzle(_v, x, y) })

// Normalisation

VECTOR vec3_i32_normalize(const VECTOR v);

// TODO: Add vec+const variations

// Field operations
#define _vector_op(a, b, c, op) a = b op c
#define _vector_iop(a, b, op) a op= b

#define _vector_op_mm(field, v0, v1, op) _vector_op(.field, (v0).field, (v1).field, op)
#define _vector_op_mc(field, v0, c, op) _vector_op(.field, (v0).field, c, op)
#define _vector_p_op_mm(field, v0, v1, op) _vector_op(.field, (v0)->field, (v1)->field, op)
#define _vector_p_op_mc(field, v0, c, op) _vector_op(.field, (v0)->field, c, op)

#define _vector_i_op_mm(field, v0, v1, op) _vector_iop((v0).field, (v1).field, op)
#define _vector_i_op_mc(field, v0, c, op) _vector_iop((v0).field, c, op)
#define _vector_ip_op_mm(field, v0, v1, op) _vector_iop((v0)->field, (v1)->field, op)
#define _vector_ip_op_mc(field, v0, c, op) _vector_iop((v0)->field, c, op)

// VECTOR - New instance

/**
 * @brief vector_op - Apply a given op piece-wise between two VECTOR instances, returning
 * the results in a new VECTOR instance
 * @param v0 - First VECTOR instance
 * @param v1  - Second VECTOR instance
 * @param op - Operation to perform piece-wise
 * @return A new vetor with the results of piece-wise operation
 */
#define vector_op(v0, v1, op) ({ \
    const VECTOR _v0 = (v0); \
    const VECTOR _v1 = (v1); \
    (VECTOR) { \
        _vector_op_mm(vx, _v0, _v1, op), \
        _vector_op_mm(vy, _v0, _v1, op), \
        _vector_op_mm(vz, _v0, _v1, op) \
    }; \
})
#define vector_c_op(v0, c, op) ({ \
    const VECTOR _v0 = (v0); \
    (VECTOR) { \
        _vector_op_mc(vx, _v0, c, op), \
        _vector_op_mc(vy, _v0, c, op), \
        _vector_op_mc(vz, _v0, c, op) \
    }; \
})

/**
 * @brief vector_p_op - Apply a given op piece-wise between two VECTOR* pointers, returning
 * the results in a new VECTOR instance
 * @param v0 - First VECTOR* instance
 * @param v1  - Second VECTOR* instance
 * @param op - Operation to perform piece-wise
 * @return A new vetor with the results of piece-wise operation
 */
#define vector_p_op(v0, v1, op) ({ \
    const VECTOR* _v0 = (v0); \
    const VECTOR* _v1 = (v1); \
    (VECTOR) { \
        _vector_p_op_mm(vx, _v0, _v1, op), \
        _vector_p_op_mm(vy, _v0, _v1, op), \
        _vector_p_op_mm(vz, _v0, _v1, op) \
    }; \
})
#define vector_c_p_op(v0, c, op) ({ \
    cosnt VECTOR* _v0 = (v0); \
    (VECTOR) { \
        _vector_p_op_mc(vx, _v0, c, op), \
        _vector_p_op_mc(vy, _v0, c, op), \
        _vector_p_op_mc(vz, _v0, c, op) \
    }; \
})

/**
 * @brief vector_add - Add VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance
 * @param v1 - Second VECTOR instance
 * @return A new vector with the results of piece-wise addition
 */
#define vector_add(v0, v1) vector_op(v0, v1, +)
#define vector_const_add(v0, c) vector_c_op(v0, c, +)

/**
 * @brief vector_p_add - Add VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance
 * @param v1 - Second VECTOR* instance
 * @return A new vector with the results of piece-wise addition
 */
#define vector_p_add(v0, v1) vector_p_op(v0, v1, +)
#define vector_const_p_add(v0, c) vector_c_p_op(v0, c, +)

/**
 * @brief vector_sub - Subtract VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance
 * @param v1 - Second VECTOR instance
 * @return A new vector with the results of piece-wise subtraction
 */
#define vector_sub(v0, v1) vector_op(v0, v1, -)
#define vector_const_sub(v0, c) vector_c_op(v0, c, -)

/**
 * @brief vector_p_sub - Subtract VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance
 * @param v1 - Second VECTOR* instance
 * @return A new vector with the results of piece-wise subtraction
 */
#define vector_p_sub(v0, v1) vector_p_op(v0, v1, -)
#define vector_const_p_sub(v0, c) vector_c_p_op(v0, c, -)

/**
 * @brief vector_mul - Multiply VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance 
 * @param v1 - Second VECTOR instance
 * @return A new vector with the results of piece-wise multiplication
 */
#define vector_mul(v0, v1) vector_op(v0, v1, *)
#define vector_const_mul(v0, c) vector_c_op(v0, c, *)

/**
 * @brief vector_p_mul - Multiply VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance 
 * @param v1 - Second VECTOR* instance
 * @return A new vector with the results of piece-wise multiplication
 */
#define vector_p_mul(v0, v1) vector_p_op(v0, v1, *)
#define vector_const_p_mul(v0, c) vector_c_p_op(v0, c, *)

/**
 * @brief vector_div - Divide VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance
 * @param v1 - Second VECTOR instance 
 * @return A new vector with the results of piece-wise division
 */
#define vector_div(v0, v1) vector_op(v0, v1, /)
#define vector_const_div(v0, c) vector_c_op(v0, c, /)

/**
 * @brief vector_p_div - Divide VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance
 * @param v1 - Second VECTOR* instance 
 * @return A new vector with the results of piece-wise division
 */
#define vector_p_div(v0, v1) vector_p_op(v0, v1, /)
#define vector_const_p_div(v0, c) vector_c_p_op(v0, c, /)

VECTOR rotationToDirection(const VECTOR* rotation);

// VECTOR - Inline

// TODO: Finish macro docstrings

#define vector_i_op(v0, v1, op) ({ \
    do { \
        _vector_i_op_mm(vx, v0, v1, op); \
        _vector_i_op_mm(vy, v0, v1, op); \
        _vector_i_op_mm(vz, v0, v1, op); \
    } while (0); \
})
#define vector_ip_op(v0, v1, op) ({ \
    do { \
        _vector_ip_op_mm(vx, v0, v1, op); \
        _vector_ip_op_mm(vy, v0, v1, op); \
        _vector_ip_op_mm(vz, v0, v1, op); \
    } while (0); \
})
#define vector_c_i_op(v0, c, op) ({ \
    do { \
        _vector_i_op_mc(vx, v0, c, op); \
        _vector_i_op_mc(vy, v0, c, op); \
        _vector_i_op_mc(vz, v0, c, op); \
    } while (0); \
})
#define vector_c_ip_op(v0, c, op) ({ \
    do { \
        _vector_ip_op_mc(vx, v0, c, op); \
        _vector_ip_op_mc(vy, v0, c, op); \
        _vector_ip_op_mc(vz, v0, c, op); \
    } while (0); \
})

#define vector_i_add(v0, v1) vector_i_op(v0, v1, +)
#define vector_i_sub(v0, v1) vector_i_op(v0, v1, -)
#define vector_i_mul(v0, v1) vector_i_op(v0, v1, *)
#define vector_i_div(v0, v1) vector_i_op(v0, v1, /)

#define vector_const_i_add(v0, c) vector_c_i_op(v0, c, +)
#define vector_const_i_sub(v0, c) vector_c_i_op(v0, c, -)
#define vector_const_i_mul(v0, c) vector_c_i_op(v0, c, *)
#define vector_const_i_div(v0, c) vector_c_i_op(v0, c, /)

#define vector_ip_add(v0, v1) vector_ip_op(v0, v1, +)
#define vector_ip_sub(v0, v1) vector_ip_op(v0, v1, -)
#define vector_ip_mul(v0, v1) vector_ip_op(v0, v1, *)
#define vector_ip_div(v0, v1) vector_ip_op(v0, v1, /)

#define vector_const_ip_add(v0, c) vector_c_ip_op(v0, c, +)
#define vector_const_ip_sub(v0, c) vector_c_ip_op(v0, c, -)
#define vector_const_ip_mul(v0, c) vector_c_ip_op(v0, c, *)
#define vector_const_ip_div(v0, c) vector_c_ip_op(v0, c, /)

// SVECTOR - New instance

#define svector_op(v0, v1, op) ({ \
    const SVECTOR _v0 = (v0); \
    const SVECTOR _v1 = (v1); \
    (SVECTOR) { \
        _vector_op_mm(vx, _v0, _v1, op), \
        _vector_op_mm(vy, _v0, _v1, op), \
        _vector_op_mm(vz, _v0, _v1, op), \
        _vector_op_mm(pad, _v0, _v1, op) \
    }; \
})
#define svector_p_op(v0, v1, op) ({ \
    const VECTOR* _v0 = (v0); \
    const VECTOR* _v1 = (v1); \
    (SVECTOR) { \
        _vector_p_op_mm(vx, _v0, _v1, op), \
        _vector_p_op_mm(vy, _v0, _v1, op), \
        _vector_p_op_mm(vz, _v0, _v1, op), \
        _vector_p_op_mm(pad, _v0, _v1, op) \
    }; \
})
#define svector_c_op(v0, c, op) ({ \
    const SVECTOR _v0 = (v0); \
    (SVECTOR) { \
        _vector_op_mc(vx, _v0, c, op), \
        _vector_op_mc(vy, _v0, c, op), \
        _vector_op_mc(vz, _v0, c, op), \
        _vector_op_mc(pad, _v0, c, op) \
    }; \
})
#define svector_c_p_op(v0, c, op) ({ \
    const SVECTOR _v0 = (v0); \
    (SVECTOR) { \
        _vector_p_op_mc(vx, _v0, c, op), \
        _vector_p_op_mc(vy, _v0, c, op), \
        _vector_p_op_mc(vz, _v0, c, op), \
        _vector_p_op_mc(pad, _v0, c, op) \
    }; \
})

#define svector_add(v0, v1) svector_op(v0, v1, +)
#define svector_sub(v0, v1) svector_op(v0, v1, -)
#define svector_mul(v0, v1) svector_op(v0, v1, *)
#define svector_div(v0, v1) svector_op(v0, v1, /)

#define svector_const_add(v0, c) svector_c_op(v0, c, +)
#define svector_const_sub(v0, c) svector_c_op(v0, c, -)
#define svector_const_mul(v0, c) svector_c_op(v0, c, *)
#define svector_const_div(v0, c) svector_c_op(v0, c, /)

#define svector_p_add(v0, v1) svector_p_op(v0, v1, +)
#define svector_p_sub(v0, v1) svector_p_op(v0, v1, -)
#define svector_p_mul(v0, v1) svector_p_op(v0, v1, *)
#define svector_p_div(v0, v1) svector_p_op(v0, v1, /)

#define svector_const_p_add(v0, c) svector_c_p_op(v0, c, +)
#define svector_const_p_sub(v0, c) svector_c_p_op(v0, c, -)
#define svector_const_p_mul(v0, c) svector_c_p_op(v0, c, *)
#define svector_const_p_div(v0, c) svector_c_p_op(v0, c, /)

// SVECTOR - Inline

#define svector_i_op(v0, v1, op) ({ \
    do { \
        _vector_i_op_mm(vx, v0, v1, op); \
        _vector_i_op_mm(vy, v0, v1, op); \
        _vector_i_op_mm(vz, v0, v1, op); \
        _vector_i_op_mm(pad, v0, v1, op); \
    } while (0); \
})
#define svector_ip_op(v0, v1, op) ({ \
    do { \
        _vector_ip_op_mm(vx, v0, v1, op); \
        _vector_ip_op_mm(vy, v0, v1, op); \
        _vector_ip_op_mm(vz, v0, v1, op); \
        _vector_ip_op_mm(pad, v0, v1, op); \
    } while (0); \
})
#define svector_c_i_op(v0, c, op) ({ \
    do { \
        _vector_i_op_mc(vx, v0, c, op); \
        _vector_i_op_mc(vy, v0, c, op); \
        _vector_i_op_mc(vz, v0, c, op); \
        _vector_i_op_mc(pad, v0, c, op); \
    } while (0); \
})
#define svector_c_ip_op(v0, c, op) ({ \
    do { \
        _vector_ip_op_mc(vx, v0, c, op); \
        _vector_ip_op_mc(vy, v0, c, op); \
        _vector_ip_op_mc(vz, v0, c, op); \
        _vector_ip_op_mc(pad, v0, c, op); \
    } while (0); \
})

#define svector_i_add(v0, v1) svector_i_op(v0, v1, +)
#define svector_i_sub(v0, v1) svector_i_op(v0, v1, -)
#define svector_i_mul(v0, v1) svector_i_op(v0, v1, *)
#define svector_i_div(v0, v1) svector_i_op(v0, v1, /)

#define svector_const_i_add(v0, c) svector_c_i_op(v0, c, +)
#define svector_const_i_sub(v0, c) svector_c_i_op(v0, c, -)
#define svector_const_i_mul(v0, c) svector_c_i_op(v0, c, *)
#define svector_const_i_div(v0, c) svector_c_i_op(v0, c, /)

#define svector_ip_add(v0, v1) svector_ip_op(v0, v1, +)
#define svector_ip_sub(v0, v1) svector_ip_op(v0, v1, -)
#define svector_ip_mul(v0, v1) svector_ip_op(v0, v1, *)
#define svector_ip_div(v0, v1) svector_ip_op(v0, v1, /)

#define svector_const_ip_add(v0, c) svector_c_ip_op(v0, c, +)
#define svector_const_ip_sub(v0, c) svector_c_ip_op(v0, c, -)
#define svector_const_ip_mul(v0, c) svector_c_ip_op(v0, c, *)
#define svector_const_ip_div(v0, c) svector_c_ip_op(v0, c, /)

// CVECTOR - New instance

#define cvector_op(v0, v1, op) ({ \
    const CVECTOR _v0 = (v0); \
    const CVECTOR _v1 = (v1); \
    (CVECTOR) { \
        _vector_op_mm(r, _v0, _v1, op), \
        _vector_op_mm(g, _v0, _v1, op), \
        _vector_op_mm(g, _v0, _v1, op), \
        _vector_op_mm(cd, _v0, _v1, op) \
    };\
})
#define cvector_p_op(v0, v1, op) ({ \
    const CVECTOR* _v0 = (v0); \
    const CVECTOR* _v1 = (v1); \
    (CVECTOR) { \
        _vector_p_op_mm(r, _v0, _v1, op), \
        _vector_p_op_mm(g, _v0, _v1, op), \
        _vector_p_op_mm(g, _v0, _v1, op), \
        _vector_p_op_mm(cd, _v0, _v1, op) \
    }; \
})
#define cvector_c_op(v0, c, op) ({ \
    const CVECTOR _v0 = (v0); \
    (CVECTOR) { \
        _vector_op_mc(r, _v0, c, op), \
        _vector_op_mc(g, _v0, c, op), \
        _vector_op_mc(g, _v0, c, op), \
        _vector_op_mc(cd, _v0, c, op) \
    }; \
})
#define cvector_c_p_op(v0, c, op) ({ \
    const CVECTOR _v0 = (v0); \
    (CVECTOR) { \
        _vector_p_op_mc(r, _v0, c, op), \
        _vector_p_op_mc(g, _v0, c, op), \
        _vector_p_op_mc(g, _v0, c, op), \
        _vector_p_op_mc(cd, _v0, c, op) \
    }; \
})

#define cvector_add(v0, v1) cvector_op(v0, v1, +)
#define cvector_sub(v0, v1) cvector_op(v0, v1, -)
#define cvector_mul(v0, v1) cvector_op(v0, v1, *)
#define cvector_div(v0, v1) cvector_op(v0, v1, /)

#define cvector_const_add(v0, c) cvector_c_op(v0, c, +)
#define cvector_const_sub(v0, c) cvector_c_op(v0, c, -)
#define cvector_const_mul(v0, c) cvector_c_op(v0, c, *)
#define cvector_const_div(v0, c) cvector_c_op(v0, c, /)

#define cvector_p_add(v0, v1) cvector_p_op(v0, v1, +)
#define cvector_p_sub(v0, v1) cvector_p_op(v0, v1, -)
#define cvector_p_mul(v0, v1) cvector_p_op(v0, v1, *)
#define cvector_p_div(v0, v1) cvector_p_op(v0, v1, /)

#define cvector_const_p_add(v0, c) cvector_c_p_op(v0, c, +)
#define cvector_const_p_sub(v0, c) cvector_c_p_op(v0, c, -)
#define cvector_const_p_mul(v0, c) cvector_c_p_op(v0, c, *)
#define cvector_const_p_div(v0, c) cvector_c_p_op(v0, c, /)

// CVECTOR - Inline

#define cvector_i_op(v0, v1, op) ({ \
    do { \
        _vector_i_op_mm(r, v0, v1, op); \
        _vector_i_op_mm(g, v0, v1, op); \
        _vector_i_op_mm(b, v0, v1, op); \
        _vector_i_op_mm(cd, v0, v1, op); \
    } while (0); \
})
#define cvector_ip_op(v0, v1, op) ({ \
    do { \
        _vector_ip_op_mm(r, v0, v1, op); \
        _vector_ip_op_mm(g, v0, v1, op); \
        _vector_ip_op_mm(b, v0, v1, op); \
        _vector_ip_op_mm(cd, v0, v1, op); \
    } while (0); \
})
#define cvector_c_i_op(v0, c, op) ({ \
    do { \
        _vector_i_op_mc(r, v0, c, op); \
        _vector_i_op_mc(g, v0, c, op); \
        _vector_i_op_mc(b, v0, c, op); \
        _vector_i_op_mc(cd, v0, c, op); \
    } while (0); \
})
#define cvector_c_ip_op(v0, c, op) ({ \
    do { \
        _vector_ip_op_mc(r, v0, c, op); \
        _vector_ip_op_mc(g, v0, c, op); \
        _vector_ip_op_mc(b, v0, c, op); \
        _vector_ip_op_mc(cd, v0, c, op); \
    } while (0); \
})

#define cvector_i_add(v0, v1) cvector_i_op(v0, v1, +)
#define cvector_i_sub(v0, v1) cvector_i_op(v0, v1, -)
#define cvector_i_mul(v0, v1) cvector_i_op(v0, v1, *)
#define cvector_i_div(v0, v1) cvector_i_op(v0, v1, /)

#define cvector_const_i_add(v0, c) cvector_c_i_op(v0, c, +)
#define cvector_const_i_sub(v0, c) cvector_c_i_op(v0, c, -)
#define cvector_const_i_mul(v0, c) cvector_c_i_op(v0, c, *)
#define cvector_const_i_div(v0, c) cvector_c_i_op(v0, c, /)

#define cvector_ip_add(v0, v1) cvector_ip_op(v0, v1, +)
#define cvector_ip_sub(v0, v1) cvector_ip_op(v0, v1, -)
#define cvector_ip_mul(v0, v1) cvector_ip_op(v0, v1, *)
#define cvector_ip_div(v0, v1) cvector_ip_op(v0, v1, /)

#define cvector_const_ip_add(v0, c) cvector_c_ip_op(v0, c, +)
#define cvector_const_ip_sub(v0, c) cvector_c_ip_op(v0, c, -)
#define cvector_const_ip_mul(v0, c) cvector_c_ip_op(v0, c, *)
#define cvector_const_ip_div(v0, c) cvector_c_ip_op(v0, c, /)

// DVECTOR - New instance

#define dvector_op(v0, v1, op) ({ \
    const DVECTOR _v0 = (v0); \
    const DVECTOR _v1 = (v1); \
    (DVECTOR) { \
        _vector_op_mm(r, _v0, _v1, op), \
        _vector_op_mm(g, _v0, _v1, op), \
    }; \
})
#define dvector_p_op(v0, v1, op) ({ \
    const DVECTOR* _v0 = (v0); \
    const DVECTOR* _v1 = (v1); \
    (DVECTOR) { \
        _vector_p_op_mm(r, _v0, _v1, op), \
        _vector_p_op_mm(g, _v0, _v1, op), \
    }; \
})
#define dvector_c_op(v0, c, op) ({ \
    const DVECTOR _v0 = (v0); \
    (DVECTOR) { \
        _vector_op_mc(r, _v0, c, op), \
        _vector_op_mc(g, _v0, c, op), \
    }; \
})
#define dvector_c_p_op(v0, c, op) ({ \
    const DVECTOR* _v0 = (v0); \
    (DVECTOR) { \
        _vector_p_op_mc(r, _v0, c, op), \
        _vector_p_op_mc(g, _v0, c, op), \
    }; \
})

#define dvector_add(v0, v1) dvector_op(v0, v1, +)
#define dvector_sub(v0, v1) dvector_op(v0, v1, -)
#define dvector_mul(v0, v1) dvector_op(v0, v1, *)
#define dvector_div(v0, v1) dvector_op(v0, v1, /)

#define dvector_const_add(v0, c) dvector_c_op(v0, c, +)
#define dvector_const_sub(v0, c) dvector_c_op(v0, c, -)
#define dvector_const_mul(v0, c) dvector_c_op(v0, c, *)
#define dvector_const_div(v0, c) dvector_c_op(v0, c, /)

#define dvector_p_add(v0, v1) dvector_p_op(v0, v1, +)
#define dvector_p_sub(v0, v1) dvector_p_op(v0, v1, -)
#define dvector_p_mul(v0, v1) dvector_p_op(v0, v1, *)
#define dvector_p_div(v0, v1) dvector_p_op(v0, v1, /)

#define dvector_const_p_add(v0, c) dvector_c_p_op(v0, c, +)
#define dvector_const_p_sub(v0, c) dvector_c_p_op(v0, c, -)
#define dvector_const_p_mul(v0, c) dvector_c_p_op(v0, c, *)
#define dvector_const_p_div(v0, c) dvector_c_p_op(v0, c, /)

// DVECTOR - Inline

#define dvector_i_op(v0, v1, op) ({ \
    do { \
        _vector_i_op_mm(r, v0, v1, op); \
        _vector_i_op_mm(g, v0, v1, op); \
    } while (0); \
})
#define dvector_ip_op(v0, v1, op) ({ \
    do { \
        _vector_ip_op_mm(r, v0, v1, op); \
        _vector_ip_op_mm(g, v0, v1, op); \
    } while (0); \
})
#define dvector_c_i_op(v0, c, op) ({ \
    do { \
        _vector_i_op_mc(r, v0, c, op); \
        _vector_i_op_mc(g, v0, c, op); \
    } while (0); \
})
#define dvector_c_ip_op(v0, c, op) ({ \
    do { \
        _vector_ip_op_mc(r, v0, c, op); \
        _vector_ip_op_mc(g, v0, c, op); \
    } while (0); \
})

#define dvector_i_add(v0, v1) dvector_i_op(v0, v1, +)
#define dvector_i_sub(v0, v1) dvector_i_op(v0, v1, -)
#define dvector_i_mul(v0, v1) dvector_i_op(v0, v1, *)
#define dvector_i_div(v0, v1) dvector_i_op(v0, v1, /)

#define dvector_const_i_add(v0, c) dvector_c_i_op(v0, c, +)
#define dvector_const_i_sub(v0, c) dvector_c_i_op(v0, c, -)
#define dvector_const_i_mul(v0, c) dvector_c_i_op(v0, c, *)
#define dvector_const_i_div(v0, c) dvector_c_i_op(v0, c, /)

#define dvector_ip_add(v0, v1) dvector_ip_op(v0, v1, +)
#define dvector_ip_sub(v0, v1) dvector_ip_op(v0, v1, -)
#define dvector_ip_mul(v0, v1) dvector_ip_op(v0, v1, *)
#define dvector_ip_div(v0, v1) dvector_ip_op(v0, v1, /)

#define dvector_const_ip_add(v0, c) dvector_c_ip_op(v0, c, +)
#define dvector_const_ip_sub(v0, c) dvector_c_ip_op(v0, c, -)
#define dvector_const_ip_mul(v0, c) dvector_c_ip_op(v0, c, *)
#define dvector_const_ip_div(v0, c) dvector_c_ip_op(v0, c, /)

#endif //MATH_UTILS_H
