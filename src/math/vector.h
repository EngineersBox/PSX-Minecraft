#pragma once

#ifndef _PSX_MINECRAFT__MATH__VECTOR_H_
#define _PSX_MINECRAFT__MATH__VECTOR_H_

#include <psxgte.h>

#include "fixed_point.h"
#include "../util/inttypes.h"

// Byte vector
typedef struct _BVECTOR {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t pad;
} BVECTOR;

// Long vector
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

#define applyGeometryMatrix(mat, vec) ({ \
    __typeof__(mat) _m = (mat); \
    __typeof__(vec) _v = (vec); \
    vec3_i32( \
        (fixedMul((i32) _m.m[0][0], _v.vx) + fixedMul((i32) _m.m[0][1], _v.vy) + fixedMul((i32) _m.m[0][2], _v.vz)) + ((i32) _m.t[0] << 12), \
        (fixedMul((i32) _m.m[1][0], _v.vx) + fixedMul((i32) _m.m[1][1], _v.vy) + fixedMul((i32) _m.m[1][2], _v.vz)) + ((i32) _m.t[1] << 12), \
        (fixedMul((i32) _m.m[2][0], _v.vx) + fixedMul((i32) _m.m[2][1], _v.vy) + fixedMul((i32) _m.m[2][2], _v.vz)) + ((i32) _m.t[2] << 12) \
    ); \
})

// Inverse of a rotation matrix is equivalent to transpose
MATRIX* InvRotMatrix(const SVECTOR* r, MATRIX* m);

// Vector init

#define _vec2_layout(x, y) .vx = (x), .vy = (y)
#define _vec3_layout(x, y, z) _vec2_layout(x, y), .vz = (z)
#define vec3_i64(x, y, z) ((LVECTOR) { _vec3_layout(x, y, z) })
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

// Comparison

#define vec3_equal(v0, v1) ((v0).vx == (v1).vx && (v0).vy == (v1).vy && (v0).vz == (v1).vz)

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
    const __typeof__(v0) _v0 = (v0); \
    const __typeof__(v1) _v1 = (v1); \
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
// High accuracy conversion using 5th order polynomial
// approximations for sin & cosine.
VECTOR rotationToDirection5o(const VECTOR* rotation);

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

// ==== COMMON DEFINITIONS ====

extern const SVECTOR VEC3_I16_ZERO;

#endif // _PSX_MINECRAFT__MATH__VECTOR_H_
