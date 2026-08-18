#include "strings.h"

#include <stdbool.h>

#include "../math/math_utils.h"

void stringToReadable(const char* src,
                      size_t src_len,
                      char* dst,
                      const size_t dst_len) {
    bool capitalise_next = true;
    for (size_t i = 0; i < min(src_len, dst_len); i++) {
        char c = src[i];
        if (c == '_') {
            capitalise_next = true;
            dst[i] = ' ';
            continue;
        } else if (capitalise_next) {
            // Each lowercase letter is 32 + uppercase equivalent.
            // Flipping 5th bit inverts case.
            dst[i] = c >= 'a' && c <= 'z'
                ? c ^ 0x20
                : c;
            capitalise_next = false;
            continue;
        }
        dst[i] = c;
    }
}
