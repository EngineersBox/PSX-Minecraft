#include "chunk_visibility.h"

u8 chunkVisibilityCalculateOffset(const u8 a, const u8 b) {
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
    return(((n - 1) * n) >> 1) + m;
}
