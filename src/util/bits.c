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