#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>
#include <psxgte.h>

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
#define cmp(a, b) (((b) > (a)) - ((b) < (a)))

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
#define absv(v) ((v) * sign(v))

#define positiveModulo(i, n) (((i) % (n) + (n)) % (n))
// __attribute__((always_inline))
// static int32_t positiveModulo(const int32_t i, const int32_t n) {
//     return (((i % n) + n) % n);
// }

static void crossProduct(const SVECTOR *v0, const SVECTOR *v1, VECTOR *out) {
    out->vx = ((v0->vy * v1->vz) - (v0->vz * v1->vy)) >> 12;
    out->vy = ((v0->vz * v1->vx) - (v0->vx * v1->vz)) >> 12;
    out->vz = ((v0->vx * v1->vy) - (v0->vy * v1->vx)) >> 12;
}

typedef struct _BVECTOR {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t pad;
} BVECTOR;

#define inlineVec(vec) (vec).vx, (vec).vy, (vec).vz
#define inlineVecPtr(vec) (vec)->vx, (vec)->vy, (vec)->vz

// Fixed point

#define FIXED_MASK_FRACTIONAL 0xffff
#define FIXED_MASK_WHOLE (~FIXED_MASK_FRACTIONAL)
#define fixedGetFractional(value) ((value) & FIXED_MASK_FRACTIONAL)
#define fixedGetWhole(value) ((value) >> FIXED_POINT_SHIFT)

/**
 * @brief Divides two fixed point int16_t numbers as x / y
 * @param x - int16_t dividend (number being divided)
 * @param y - int16_t divisor (number performing division)
 * @return Result of division x / y
 */
#define fixedDiv(x, y) (int16_t)((((int32_t)x << FIXED_POINT_SHIFT) / y) >> FIXED_POINT_SHIFT)

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

/**
 * @brief Multiplies two fixed point int16_t numbers as x * y
 * @param x - int16_t left side of multiplication
 * @param y - in16_t right side of multiplication
 * @return Result of multiplication x * y
 */
#define fixedMul(x, y) (int16_t)(((int32_t)x * (int32_t)y) / ONE)

// TODO: Add vec+const variations

// Field operations
#define _vec_op_single(field, v0, v1, op) .field = (v0).field op (v1).field
#define _vec_ptr_op_single(field, v0, v1, op) .field = (v0)->field op (v1)->field
#define _ivec_op_single(field, v0, v1, op) (v0).field op= (v1).field
#define _ivec_ptr_op_single(field, v0, v1, op) (v0)->field op= (v1)->field

// VECTOR - New instance

/**
 * @brief vop - Apply a given op piece-wise between two VECTOR instances, returning
 * the results in a new VECTOR instance
 * @param v0 - First VECTOR instance
 * @param v1  - Second VECTOR instance
 * @param op - Operation to perform piece-wise
 * @return A new vetor with the results of piece-wise operation
 */
#define vop(v0, v1, op) ((VECTOR) { \
    _vec_op_single(vx, v0, v1, op), \
    _vec_op_single(vy, v0, v1, op), \
    _vec_op_single(vz, v0, v1, op) \
})

/**
 * @brief vpop - Apply a given op piece-wise between two VECTOR* pointers, returning
 * the results in a new VECTOR instance
 * @param v0 - First VECTOR* instance
 * @param v1  - Second VECTOR* instance
 * @param op - Operation to perform piece-wise
 * @return A new vetor with the results of piece-wise operation
 */
#define vpop(v0, v1, op) ((VECTOR) { \
    _vec_ptr_op_single(vx, v0, v1, op), \
    _vec_ptr_op_single(vy, v0, v1, op), \
    _vec_ptr_op_single(vz, v0, v1, op) \
})

/**
 * @brief vadd - Add VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance
 * @param v1 - Second VECTOR instance
 * @return A new vector with the results of piece-wise addition
 */
#define vadd(v0, v1) vop(v0, v1, +)

/**
 * @brief vpadd - Add VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance
 * @param v1 - Second VECTOR* instance
 * @return A new vector with the results of piece-wise addition
 */
#define vpadd(v0, v1) vpop(v0, v1, +)

/**
 * @brief vsub - Subtract VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance
 * @param v1 - Second VECTOR instance
 * @return A new vector with the results of piece-wise subtraction
 */
#define vsub(v0, v1) vop(v0, v1, -)

/**
 * @brief vpsub - Subtract VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance
 * @param v1 - Second VECTOR* instance
 * @return A new vector with the results of piece-wise subtraction
 */
#define vpsub(v0, v1) vpop(v0, v1, -)

/**
 * @brief vmul - Multiply VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance 
 * @param v1 - Second VECTOR instance
 * @return A new vector with the results of piece-wise multiplication
 */
#define vmul(v0, v1) vop(v0, v1, *)

/**
 * @brief vpmul - Multiply VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance 
 * @param v1 - Second VECTOR* instance
 * @return A new vector with the results of piece-wise multiplication
 */
#define vpmul(v0, v1) vpop(v0, v1, *)

/**
 * @brief vdiv - Divide VECTORs v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR instance
 * @param v1 - Second VECTOR instance 
 * @return A new vector with the results of piece-wise division
 */
#define vdiv(v0, v1) vop(v0, v1, /)

/**
 * @brief vpdiv - Divide VECTOR pointers v0 and v1 together piece-wise to form
 * a new vector instance and return it. This does
 * not modify the original vectors
 * @param v0 - First VECTOR* instance
 * @param v1 - Second VECTOR* instance 
 * @return A new vector with the results of piece-wise division
 */
#define vpdiv(v0, v1) vpop(v0, v1, /)

VECTOR rotationToDirection(const VECTOR* rotation);

// VECTOR - Inline

// TODO: Finish macro docstrings

#define viop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(vx, v0, v1, op); \
        _ivec_op_single(vy, v0, v1, op); \
        _ivec_op_single(vz, v0, v1, op); \
    } while (0); \
})

#define viadd(v0, v1) viop(v0, v1, +)
#define visub(v0, v1) viop(v0, v1, -)
#define vimul(v0, v1) viop(v0, v1, *)
#define vidiv(v0, v1) viop(v0, v1, /)

// SVECTOR - New instance

#define svop(v0, v1, op) ((SVECTOR) { \
    _vec_op_single(vx, v0, v1, op), \
    _vec_op_single(vy, v0, v1, op), \
    _vec_op_single(vz, v0, v1, op), \
    _vec_op_single(pad, v0, v1, op) \
})

#define svadd(v0, v1) svop(v0, v1, +)
#define svsub(v0, v1) svop(v0, v1, -)
#define svmul(v0, v1) svop(v0, v1, *)
#define svdiv(v0, v1) svop(v0, v1, /)

// SVECTOR - Inline

#define sviop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(vx, v0, v1, op); \
        _ivec_op_single(vy, v0, v1, op); \
        _ivec_op_single(vz, v0, v1, op); \
        _ivec_op_single(pad, v0, v1, op); \
    } while (0); \
})

#define sviadd(v0, v1) sviop(v0, v1, +)
#define svisub(v0, v1) sviop(v0, v1, -)
#define svimul(v0, v1) sviop(v0, v1, *)
#define svidiv(v0, v1) sviop(v0, v1, /)

// CVECTOR - New instance

#define cvop(v0, v1, op) ((CVECTOR) { \
    _vec_op_single(r, v0, v1, op), \
    _vec_op_single(g, v0, v1, op), \
    _vec_op_single(g, v0, v1, op), \
    _vec_op_single(cd, v0, v1, op) \
})

#define cvadd(v0, v1) cvop(v0, v1, +)
#define cvsub(v0, v1) cvop(v0, v1, -)
#define cvmul(v0, v1) cvop(v0, v1, *)
#define cvdiv(v0, v1) cvop(v0, v1, /)

// CVECTOR - Inline

#define cviop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(r, v0, v1, op); \
        _ivec_op_single(g, v0, v1, op); \
        _ivec_op_single(b, v0, v1, op); \
        _ivec_op_single(cd, v0, v1, op); \
    } while (0); \
})

#define cviadd(v0, v1) cviop(v0, v1, +)
#define cvisub(v0, v1) cviop(v0, v1, -)
#define cvimul(v0, v1) cviop(v0, v1, *)
#define cvidiv(v0, v1) cviop(v0, v1, /)

// DVECTOR - New instance

#define dvop(v0, v1, op) ((DVECTOR) { \
    _vec_op_single(r, v0, v1, op), \
    _vec_op_single(g, v0, v1, op), \
})

#define dvadd(v0, v1) dvop(v0, v1, +)
#define dvsub(v0, v1) dvop(v0, v1, -)
#define dvmul(v0, v1) dvop(v0, v1, *)
#define dvdiv(v0, v1) dvop(v0, v1, /)

// DVECTOR - Inline

#define dviop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(r, v0, v1, op); \
        _ivec_op_single(g, v0, v1, op); \
    } while (0); \
})

#define dviadd(v0, v1) dviop(v0, v1, +)
#define dvisub(v0, v1) dviop(v0, v1, -)
#define dvimul(v0, v1) dviop(v0, v1, *)
#define dvidiv(v0, v1) dviop(v0, v1, /)

#endif //MATH_UTILS_H
