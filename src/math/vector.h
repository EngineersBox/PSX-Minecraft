#pragma once

#ifndef _PSXMC__MATH__VECTOR_H_
#define _PSXMC__MATH__VECTOR_H_

#include <psxgte.h>

#include "fixed_point.h"
#include "../util/inttypes.h"

// Byte vector
typedef struct _BVECTOR {
    u8 vx;
    u8 vy;
    u8 vz;
    u8 pad;
} BVECTOR;

typedef struct _IBVECTOR {
    i8 vx;
    i8 vy;
    i8 vz;
    i8 pad;
} IBVECTOR;

typedef struct _BDVECTOR {
    u8 u;
    u8 v;
} BDVECTOR;

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
#define vec3_i8(x, y, z) ((IBVECTOR) { _vec3_layout(x, y, z) })
#define vec3_u8(x, y, z) ((BVECTOR) { _vec3_layout(x, y, z) })
#define vec2_i16(x, y) ((DVECTOR) { _vec2_layout(x, y) })
#define vec3_rgb(_r, _g, _b) ((CVECTOR) { .r = (_r), .g = (_b), .b = (_b) })

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
#define vec3_swizzle(_v, x, y, z) ((__typeof__(_v)) { _vec3_layout_swizzle(_v, x, y, z) })
#define vec2_swizzle(_v, x, y) ((__typeof__(v)) { _vec2_layout_swizzle(_v, x, y) })

// Normalisation

VECTOR vec3_i32_normalize(const VECTOR v);

// Comparison

#define vec2_equal(v0, v1) ((v0).vx == (v1).vx && (v0).vy == (v1).vy)
#define vec3_equal(v0, v1) (vec2_equal(v0, v1) && (v0).vz == (v1).vz)

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

// Vector varaint operation bindings

#define vec3_op(v0, op, v1) ({ \
    const __typeof__(v0) _v0 = (v0); \
    const __typeof__(v1) _v1 = (v1); \
    (__typeof__(v0)) { \
        _vector_op_mm(vx, _v0, _v1, op), \
        _vector_op_mm(vy, _v0, _v1, op), \
        _vector_op_mm(vz, _v0, _v1, op) \
    }; \
})
#define vec3_const_op(v0, op, c) ({ \
    const __typeof__(v0) _v0 = (v0); \
    (__typeof__(v0)) { \
        _vector_op_mc(vx, _v0, c, op), \
        _vector_op_mc(vy, _v0, c, op), \
        _vector_op_mc(vz, _v0, c, op) \
    }; \
})

#define pvec3_op(v0, op, v1) ({ \
    const __typeof__(v0)* _v0 = (v0); \
    const __typeof__(v1)* _v1 = (v1); \
    (__typeof__(v0)) { \
        _vector_p_op_mm(vx, _v0, _v1, op), \
        _vector_p_op_mm(vy, _v0, _v1, op), \
        _vector_p_op_mm(vz, _v0, _v1, op) \
    }; \
})
#define pvec3_const_op(v0, op, c) ({ \
    cosnt __typeof__(v0)* _v0 = (v0); \
    (__typeof__(v0)) { \
        _vector_p_op_mc(vx, _v0, c, op), \
        _vector_p_op_mc(vy, _v0, c, op), \
        _vector_p_op_mc(vz, _v0, c, op) \
    }; \
})

#define vec2_op(v0, op, v1) ({ \
    const __typeof__(v0) _v0 = (v0); \
    const __typeof__(v1) _v1 = (v1); \
    (__typeof__(v0)) { \
        _vector_op_mm(vx, _v0, _v1, op), \
        _vector_op_mm(vy, _v0, _v1, op) \
    }; \
})
#define vec2_const_op(v0, op, c) ({ \
    const __typeof__(v0) _v0 = (v0); \
    (__typeof__(v0)) { \
        _vector_op_mc(vx, _v0, c, op), \
        _vector_op_mc(vy, _v0, c, op) \
    }; \
})

#define pvec2_op(v0, op, v1) ({ \
    const __typeof__(v0)* _v0 = (v0); \
    const __typeof__(v1)* _v1 = (v1); \
    (__typeof__(v0)) { \
        _vector_p_op_mm(vx, _v0, _v1, op), \
        _vector_p_op_mm(vy, _v0, _v1, op) \
    }; \
})
#define pvec2_const_op(v0, op, c) ({ \
    cosnt __typeof__(v0)* _v0 = (v0); \
    (__typeof__(v0)) { \
        _vector_p_op_mc(vx, _v0, c, op), \
        _vector_p_op_mc(vy, _v0, c, op) \
    }; \
})

// Operations

// - Operands vec3, vec3

#define vec3_add(v0, v1) vec3_op(v0, +, v1)
#define vec3_sub(v0, v1) vec3_op(v0, -, v1)
#define vec3_div(v0, v1) vec3_op(v0, /, v1)
#define vec3_mul(v0, v1) vec3_op(v0, *, v1)
#define vec3_lshift(v0, v1) vec3_op(v0, <<, v1)
#define vec3_rshift(v0, v1) vec3_op(v0, >>, v1)
#define vec3_mod(v0, v1) vec3_op(v0, %, v1)
#define vec3_and(v0, v1) vec3_op(v0, &, v1)
#define vec3_or(v0, v1) vec3_op(v0, |, v1)
#define vec3_xor(v0, v1) vec3_op(v0, ^, v1)

// - Operands vec3*, vec3*

#define pvec3_add(v0, v1) pvec3_op(v0, +, v1)
#define pvec3_sub(v0, v1) pvec3_op(v0, -, v1)
#define pvec3_div(v0, v1) pvec3_op(v0, /, v1)
#define pvec3_mul(v0, v1) pvec3_op(v0, *, v1)
#define pvec3_lshift(v0, v1) pvec3_op(v0, <<, v1)
#define pvec3_rshift(v0, v1) pvec3_op(v0, >>, v1)
#define pvec3_mod(v0, v1) pvec3_op(v0, %, v1)
#define pvec3_and(v0, v1) pvec3_op(v0, &, v1)
#define pvec3_or(v0, v1) pvec3_op(v0, |, v1)
#define pvec3_xor(v0, v1) pvec3_op(v0, ^, v1)

// - Operands vec3, integer

#define vec3_const_add(v0, c) vec3_const_op(v0, +, c)
#define vec3_const_sub(v0, c) vec3_const_op(v0, -, c)
#define vec3_const_div(v0, c) vec3_const_op(v0, /, c)
#define vec3_const_mul(v0, c) vec3_const_op(v0, *, c)
#define vec3_const_lshift(v0, c) vec3_const_op(v0, <<, c)
#define vec3_const_rshift(v0, c) vec3_const_op(v0, >>, c)
#define vec3_const_mod(v0, c) vec3_const_op(v0, %, c)
#define vec3_const_and(v0, c) vec3_const_op(v0, &, c)
#define vec3_const_or(v0, c) vec3_const_op(v0, |, c)
#define vec3_const_xor(v0, c) vec3_const_op(v0, ^, c)

// - Operands vec3*, integer

#define pvec3_const_add(v0, c) pvec3_const_op(v0, +, c)
#define pvec3_const_sub(v0, c) pvec3_const_op(v0, -, c)
#define pvec3_const_div(v0, c) pvec3_const_op(v0, /, c)
#define pvec3_const_mul(v0, c) pvec3_const_op(v0, *, c)
#define pvec3_const_lshift(v0, c) pvec3_const_op(v0, <<, c)
#define pvec3_const_rshift(v0, c) pvec3_const_op(v0, >>, c)
#define pvec3_const_mod(v0, c) pvec3_const_op(v0, %, c)
#define pvec3_const_and(v0, c) pvec3_const_op(v0, &, c)
#define pvec3_const_or(v0, c) pvec3_const_op(v0, |, c)
#define pvec3_const_xor(v0, c) pvec3_const_op(v0, ^, c)

// - Operands vec2, vec2

#define vec2_add(v0, v1) vec2_op(v0, +, v1)
#define vec2_sub(v0, v1) vec2_op(v0, -, v1)
#define vec2_div(v0, v1) vec2_op(v0, /, v1)
#define vec2_mul(v0, v1) vec2_op(v0, *, v1)
#define vec2_lshift(v0, v1) vec2_op(v0, <<, v1)
#define vec2_rshift(v0, v1) vec2_op(v0, >>, v1)
#define vec2_mod(v0, v1) vec2_op(v0, %, v1)
#define vec2_and(v0, v1) vec2_op(v0, &, v1)
#define vec2_or(v0, v1) vec2_op(v0, |, v1)
#define vec2_xor(v0, v1) vec2_op(v0, ^, v1)

// - Operands vec2*, vec2*

#define pvec2_add(v0, v1) pvec2_op(v0, +, v1)
#define pvec2_sub(v0, v1) pvec2_op(v0, -, v1)
#define pvec2_div(v0, v1) pvec2_op(v0, /, v1)
#define pvec2_mul(v0, v1) pvec2_op(v0, *, v1)
#define pvec2_lshift(v0, v1) pvec2_op(v0, <<, v1)
#define pvec2_rshift(v0, v1) pvec2_op(v0, >>, v1)
#define pvec2_mod(v0, v1) pvec2_op(v0, %, v1)
#define pvec2_and(v0, v1) pvec2_op(v0, &, v1)
#define pvec2_or(v0, v1) pvec2_op(v0, |, v1)
#define pvec2_xor(v0, v1) pvec2_op(v0, ^, v1)

// - Operands vec2, integer

#define vec2_const_add(v0, c) vec2_const_op(v0, +, c)
#define vec2_const_sub(v0, c) vec2_const_op(v0, -, c)
#define vec2_const_div(v0, c) vec2_const_op(v0, /, c)
#define vec2_const_mul(v0, c) vec2_const_op(v0, *, c)
#define vec2_const_lshift(v0, c) vec2_const_op(v0, <<, c)
#define vec2_const_rshift(v0, c) vec2_const_op(v0, >>, c)
#define vec2_const_mod(v0, c) vec2_const_op(v0, %, c)
#define vec2_const_and(v0, c) vec2_const_op(v0, &, c)
#define vec2_const_or(v0, c) vec2_const_op(v0, |, c)
#define vec2_const_xor(v0, c) vec2_const_op(v0, ^, c)

// - Operands vec2*, integer

#define pvec2_const_add(v0, c) pvec2_const_op(v0, +, c)
#define pvec2_const_sub(v0, c) pvec2_const_op(v0, -, c)
#define pvec2_const_div(v0, c) pvec2_const_op(v0, /, c)
#define pvec2_const_mul(v0, c) pvec2_const_op(v0, *, c)
#define pvec2_const_lshift(v0, c) pvec2_const_op(v0, <<, c)
#define pvec2_const_rshift(v0, c) pvec2_const_op(v0, >>, c)
#define pvec2_const_mod(v0, c) pvec2_const_op(v0, %, c)
#define pvec2_const_and(v0, c) pvec2_const_op(v0, &, c)
#define pvec2_const_or(v0, c) pvec2_const_op(v0, |, c)
#define pvec2_const_xor(v0, c) pvec2_const_op(v0, ^, c)

// Rotations

VECTOR rotationToDirection(const VECTOR* rotation);
VECTOR rotationToDirection5o(const VECTOR* rotation);

// ==== COMMON DEFINITIONS ====

extern const VECTOR VEC3_I32_ZERO;
extern const SVECTOR VEC3_I16_ZERO;

#endif // _PSXMC__MATH__VECTOR_H_
