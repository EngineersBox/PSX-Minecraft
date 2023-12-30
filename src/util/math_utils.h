#pragma once

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>

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

// TODO: Create docstrings for all these macros

// Field operations
#define _vec_op_single(field, v0, v1, op) .field = (v0).field op (v1).field
#define _ivec_op_single(field, v0, v1, op) (v0).field op= (v1).field

// VECTOR - New instance

#define _vop(v0, v1, op) ((VECTOR) { \
    _vec_op_single(vx, v0, v1, op), \
    _vec_op_single(vy, v0, v1, op), \
    _vec_op_single(vz, v0, v1, op) \
})

#define vadd(v0, v1) _vop(v0, v1, +)
#define vsub(v0, v1) _vop(v0, v1, -)
#define vmul(v0, v1) _vop(v0, v1, *)
#define vdiv(v0, v1) _vop(v0, v1, /)

// VECTOR - Inline

#define _viop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(vx, v0, v1, op); \
        _ivec_op_single(vy, v0, v1, op); \
        _ivec_op_single(vz, v0, v1, op); \
    } while (0); \
})

#define viadd(v0, v1) _vop(v0, v1, +)
#define visub(v0, v1) _vop(v0, v1, -)
#define vimul(v0, v1) _vop(v0, v1, *)
#define vidiv(v0, v1) _vop(v0, v1, /)

// SVECTOR - New instance

#define _svop(v0, v1, op) ((SVECTOR) { \
    _vec_op_single(vx, v0, v1, op), \
    _vec_op_single(vy, v0, v1, op), \
    _vec_op_single(vz, v0, v1, op), \
    _vec_op_single(pad, v0, v1, op) \
})

#define svadd(v0, v1) _svop(v0, v1, +)
#define svsub(v0, v1) _svop(v0, v1, -)
#define svmul(v0, v1) _svop(v0, v1, *)
#define svdiv(v0, v1) _svop(v0, v1, /)

// SVECTOR - Inline

#define _sviop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(vx, v0, v1, op); \
        _ivec_op_single(vy, v0, v1, op); \
        _ivec_op_single(vz, v0, v1, op); \
        _ivec_op_single(pad, v0, v1, op); \
    } while (0); \
})

#define sviadd(v0, v1) _sviop(v0, v1, +)
#define svisub(v0, v1) _sviop(v0, v1, -)
#define svimul(v0, v1) _sviop(v0, v1, *)
#define svidiv(v0, v1) _sviop(v0, v1, /)

// CVECTOR - New instance

#define _cvop(v0, v1, op) ((CVECTOR) { \
    _vec_op_single(r, v0, v1, op), \
    _vec_op_single(g, v0, v1, op), \
    _vec_op_single(g, v0, v1, op), \
    _vec_op_single(cd, v0, v1, op) \
})

#define cvadd(v0, v1) _cvop(v0, v1, +)
#define cvsub(v0, v1) _cvop(v0, v1, -)
#define cvmul(v0, v1) _cvop(v0, v1, *)
#define cvdiv(v0, v1) _cvop(v0, v1, /)

// CVECTOR - Inline

#define _cviop(v0, v1, op) ({ \
    do { \
        _ivec_op_single(r, v0, v1, op); \
        _ivec_op_single(g, v0, v1, op); \
        _ivec_op_single(b, v0, v1, op); \
        _ivec_op_single(cd, v0, v1, op); \
    } while (0); \
})

#define sviadd(v0, v1) _cviop(v0, v1, +)
#define svisub(v0, v1) _cviop(v0, v1, -)
#define svimul(v0, v1) _cviop(v0, v1, *)
#define svidiv(v0, v1) _cviop(v0, v1, /)

#endif //MATH_UTILS_H
