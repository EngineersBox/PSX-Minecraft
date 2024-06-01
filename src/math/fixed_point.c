#include "fixed_point.h"

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
