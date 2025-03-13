#include "chunk_visibility.h"

u8 chunkVisibilityGetBit(const ChunkVisibility vis, const u8 a, const u8 b) {
    if (a == b) {
        return 0;
    }
    u8 n;
    u8 m;
    if (a > b) {
        n = a;
        m = b;
    } else {
        n = b;
        m = a;
    }
    const u8 offset = (((n - 1) * n) >> 1) + m;
    return (vis >> offset) & 0b1;
}

void chunkVisibilitySetBit(ChunkVisibility* vis, const u8 a, const u8 b) {
    if (a == b) {
        return;
    }
    u8 n;
    u8 m;
    if (a > b) {
        n = a;
        m = b;
    } else {
        n = b;
        m = a;
    }
    const u8 offset = (((n - 1) * n) >> 1) + m;
    *vis |= 0b1 << offset;
}
