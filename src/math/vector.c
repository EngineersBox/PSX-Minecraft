#include "vector.h"

#include <stdio.h>

#include "fixed_point.h"
#include "../logging/logging.h"
#include "../util/preprocessor.h"

const VECTOR VEC3_I32_ZERO = vec3_i32(0);
const SVECTOR VEC3_I16_ZERO = vec3_i16(0);

VECTOR rotationToDirection(const VECTOR* rotation) {
    // printf("Rotation: " VEC_PATTERN "\n", rotation->vx, rotation->vy, rotation->vz);
    const int32_t x = rotation->vx >> FIXED_POINT_SHIFT;
    const int32_t y = rotation->vy >> FIXED_POINT_SHIFT;
    const int32_t xz_len = icos(x);
    return vec3_i32(
        (xz_len * isin(-y)) >> FIXED_POINT_SHIFT,
        -isin(x), // Negation to conver from -Y up to +Y up coordinate space
        (xz_len * icos(y)) >> FIXED_POINT_SHIFT
);
}

VECTOR rotationToDirection5o(const VECTOR* rotation) {
    const int32_t x = rotation->vx >> TRIG_5TH_ORDER_REDUCTION;
    const int32_t y = rotation->vy >> TRIG_5TH_ORDER_REDUCTION;
    const int32_t xz_len = cos5o(x);
    return vec3_i32(
        (xz_len * sin5o(-y)) >> FIXED_POINT_SHIFT,
        -sin5o(x), // Negation to conver from -Y up to +Y up coordinate space
        (xz_len * cos5o(y)) >> FIXED_POINT_SHIFT
    );
}

VECTOR vec3_i32_normalize(const VECTOR v) {
    const fixedi32 length = SquareRoot12(dot_i32(v,v));
    /*DEBUG_LOG("[MATH] Normalize. Source: " VEC_PATTERN ", Length: %d\n", VEC_LAYOUT(v), length);*/
    return length == ONE ? v : vec3_const_div(
        vec3_const_lshift(v, FIXED_POINT_SHIFT),
        length
    );
}

MATRIX *InvRotMatrix(const SVECTOR *r, MATRIX *m) {
    i16 s[3],c[3];
    MATRIX tm[3];

    s[0] = isin(r->vx);		s[1] = isin(r->vy);		s[2] = isin(r->vz);
    c[0] = icos(r->vx);		c[1] = icos(r->vy);		c[2] = icos(r->vz);

    // mX
    // m->m[0][0] = ONE;		m->m[0][1] = 0;			m->m[0][2] = 0;
    // m->m[1][0] = 0;			m->m[1][1] = c[0];		m->m[1][2] = -s[0];
    // m->m[2][0] = 0;			m->m[2][1] = s[0];		m->m[2][2] = c[0];
    m->m[0][0] = ONE;		m->m[0][1] = 0;			m->m[0][2] = 0;
    m->m[1][0] = 0;			m->m[1][1] = c[0];		m->m[1][2] = s[0];
    m->m[2][0] = 0;			m->m[2][1] = -s[0];		m->m[2][2] = c[0];

    // mY
    // tm[0].m[0][0] = c[1];	tm[0].m[0][1] = 0;		tm[0].m[0][2] = s[1];
    // tm[0].m[1][0] = 0;		tm[0].m[1][1] = ONE;	tm[0].m[1][2] = 0;
    // tm[0].m[2][0] = -s[1];	tm[0].m[2][1] = 0;		tm[0].m[2][2] = c[1];
    tm[0].m[0][0] = c[1];	tm[0].m[0][1] = 0;		tm[0].m[0][2] = -s[1];
    tm[0].m[1][0] = 0;		tm[0].m[1][1] = ONE;	tm[0].m[1][2] = 0;
    tm[0].m[2][0] = s[1];	tm[0].m[2][1] = 0;		tm[0].m[2][2] = c[1];

    // mZ
    // tm[1].m[0][0] = c[2];	tm[1].m[0][1] = -s[2];	tm[1].m[0][2] = 0;
    // tm[1].m[1][0] = s[2];	tm[1].m[1][1] = c[2];	tm[1].m[1][2] = 0;
    // tm[1].m[2][0] = 0;		tm[1].m[2][1] = 0;		tm[1].m[2][2] = ONE;
    tm[1].m[0][0] = c[2];	tm[1].m[0][1] = s[2];	tm[1].m[0][2] = 0;
    tm[1].m[1][0] = -s[2];	tm[1].m[1][1] = c[2];	tm[1].m[1][2] = 0;
    tm[1].m[2][0] = 0;		tm[1].m[2][1] = 0;		tm[1].m[2][2] = ONE;

    PushMatrix();
    MulMatrix0(m, &tm[0], &tm[2]);
    MulMatrix0(&tm[2], &tm[1], m);
    PopMatrix();

    return m;
}

// 4096 / ((((a & 0xffff) << 12) / (b & 0xffff)) >> 12)
// ((a & 0xffff) * (b & 0xffff)) >> 12

// 0.7 * 0.2 = 0.14
