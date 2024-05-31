#include "math_utils.h"

#include <stdio.h>

#include "../logging/logging.h"

const SVECTOR VEC3_I16_ZERO = vec3_i16_all(0);

VECTOR rotationToDirection(const VECTOR* rotation) {
    // printf("Rotation: " VEC_PATTERN "\n", rotation->vx, rotation->vy, rotation->vz);
    const int32_t x = rotation->vx >> FIXED_POINT_SHIFT;
    const int32_t y = rotation->vy >> FIXED_POINT_SHIFT;
    const int32_t xz_len = icos(x);
    return (VECTOR) {
        .vx = (xz_len * isin(-y)) >> FIXED_POINT_SHIFT,
        .vy = -isin(x), // Negation to conver from -Y up to +Y up coordinate space
        .vz = (xz_len * icos(y)) >> FIXED_POINT_SHIFT
    };
}

VECTOR vec3_i32_normalize(const VECTOR v) {
    const fixedi32 length = SquareRoot12(dot_i32(v,v));
    // DEBUG_LOG("[MATH] Normalize. Source: " VEC_PATTERN ", Length: %d\n", VEC_LAYOUT(v), length);
    return vec3_i32(
        (v.vx << FIXED_POINT_SHIFT) / length,
        (v.vy << FIXED_POINT_SHIFT) / length,
        (v.vz << FIXED_POINT_SHIFT) / length
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

i32 sin5o(i16 i) {
    /* Convert (signed) input to a value between 0 and 8192. (8192 is pi/2, which is the region of the curve fit). */
    /* ------------------------------------------------------------------- */
    i <<= 1;
    u8 c = i<0; //set carry for output pos/neg

    if(i == (i|0x4000)) // flip input value to corresponding value in range [0..8192)
        i = (1<<15) - i;
    i = (i & 0x7FFF) >> 1;
    /* ------------------------------------------------------------------- */

    /* The following section implements the formula:
     = y * 2^-n * ( A1 - 2^(q-p)* y * 2^-n * y * 2^-n * [B1 - 2^-r * y * 2^-n * C1 * y]) * 2^(a-q)
    Where the constants are defined as follows:
    */
    enum {A1=3370945099UL, B1=2746362156UL, C1=292421UL};
    enum {n=13, p=32, q=31, r=3, a=12};

    u32 y = (C1*((u32)i))>>n;
    y = B1 - (((u32)i*y)>>r);
    y = (u32)i * (y>>n);
    y = (u32)i * (y>>n);
    y = A1 - (y>>(p-q));
    y = (u32)i * (y>>n);
    y = (y+(1UL<<(q-a-1)))>>(q-a); // Rounding

    return c ? -y : y;
}

i32 cos5o(const i16 i) {
    return sin5o((int16_t)(((uint16_t)(i)) + 8192U));
};

// 4096 / ((((a & 0xffff) << 12) / (b & 0xffff)) >> 12)
// ((a & 0xffff) * (b & 0xffff)) >> 12

// 0.7 * 0.2 = 0.14