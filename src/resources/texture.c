#include "texture.h"

#include "../math/math_utils.h"

// TODO: Explain all of this nonsense.

const RECT single_block_texture_window = (RECT) {
    0b0010,
    0b0010,
    0b0000,
    0b0000
};

u8 textureWindowMaskBlock(const u16 coord) {
    if (coord >= 32 && isPowerOf2(coord >> 4)) {
        return 0b00010;
    }
    return 0b11110;
}

u8 textureWindowOffsetBlock(const u16 coord) {
    if (coord >= 32 && isPowerOf2(coord >> 4)) {
        return 0b00000;
    }
    return coord >> 3;
}
