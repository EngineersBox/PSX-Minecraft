#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>

#define FIXED_POINT_SHIFT 12

#define max(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b; \
})
#define min(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b; \
})
#define clamp(x, lower, upper) (min((upper), max((x), (lower))))

#define sign(x) (((x) > 0) - ((x) < 0))

static void crossProduct(const SVECTOR* v0, const SVECTOR* v1, VECTOR* out) {
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

// Fixed point

/**
 * @brief Divides two fixed point int16_t numbers as x / y
 * @param x - int16_t dividend (number being divided)
 * @param y - int16_t divisor (number performing division)
 * @return Result of division x / y
 */
#define fixedDiv(x, y) (int16_t)(((int32_t)x << 12) / y)

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
#define vdiv(v0, v1) vop(v0, v1, /)

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

#endif //MATH_UTILS_H
