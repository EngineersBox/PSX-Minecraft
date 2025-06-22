#include "bits.h"

u8 trailing_zeros(u32 value) {
    // Ripped from: https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
    // If value is 1101000 (base 2), then c will be 3
    // NOTE: if value == 0, then c = 31.
    if (value & 0x1) {
        // special case for odd v (assumed to happen half of the time)
        return 0;
    }
    u32 c = 1; // Number of zero bits on the right
    if ((value & 0xffff) == 0) {
        value >>= 16;
        c += 16;
    }
    if ((value & 0xff) == 0) {
        value >>= 8;
        c += 8;
    }
    if ((value & 0xf) == 0) {
        value >>= 4;
        c += 4;
    }
    if ((value & 0x3) == 0) {
        value >>= 2;
        c += 2;
    }
    return c - (value & 0x1);
}

u8 trailing_ones(u32 value) {
    // Ripped from: https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
    // If value is 0010111(base 2), then c will be 3
    // NOTE: if value == 0xffffffff, then c = 31.
    if (value == 0) {
        // special case for odd v (assumed to happen half of the time)
        return 0;
    }
    u32 c = 1; // Number of zero bits on the right
    if ((value & 0xffff) == 0xffff) {
        value >>= 16;
        c += 16;
    }
    if ((value & 0xff) == 0xff) {
        value >>= 8;
        c += 8;
    }
    if ((value & 0xf) == 0xf) {
        value >>= 4;
        c += 4;
    }
    if ((value & 0x3) == 0x3) {
        value >>= 2;
        c += 2;
    }
    return c - !(value & 0x1);
}

// These functions (defined as macros in gcc/include/longlong.h are
// used as the definition for __ctzSI2 in gcc/libgcc/libgcc2.c.
// Specifically count_trailing_zeros. They seem to be the above
// implementation in a more succinct form though unsure as to whether
// the performance is better or not. If it is then we can replace
// the use of trailing_zeros with __builtin_ctz.
// Godbolt comparison: https://godbolt.org/z/ffqvYWsrr
//
// int count_leading_zeros(int x) {
//     UWtype __xr = (x);
//     UWtype __a;
//     if (W_TYPE_SIZE <= 32) { // Never true since R3000 is 32 bit.. dunno why this isn't a preprocessor #if?
//         __a = __xr < ((UWtype) 1<<2 * __BITS4)
//             ? (__xr < ((UWtype) 1 << __BITS4) ? 0 : __BITS4)
//             : (__xr < ((UWtype) 1 << 3 * __BITS4) ? 2 * __BITS4 : 3 * __BITS4);
//     } else {
//         for (__a = W_TYPE_SIZE - 8; __a > 0; __a -= 8)
//             if (((__xr >> __a) & 0xff) != 0)
//                 break;
//     }
//     return W_TYPE_SIZE - (__clz_tab[__xr >> __a] + __a);
// }
//
// int count_trailing_zeros(int x) {
//     UWtype __ctz_x = (x);
//     UWtype __ctz_c;
//     count_leading_zeros (__ctz_c, __ctz_x & -__ctz_x);
//     return W_TYPE_SIZE - 1 - __ctz_c;
// }
