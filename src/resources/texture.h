#pragma once

#ifndef PSXMC_TEXTURE_H
#define PSXMC_TEXTURE_H

#include <psxgte.h>
#include <psxgpu.h>

#include "../util/inttypes.h"
#include "../math/vector.h"

typedef struct Texture {
    u16 tpage;
    u16 clut;
} Texture;

typedef struct TextureAttributes {
    u8 u;
    u8 v;
    u8 w;
    u8 h;
    CVECTOR tint;
} TextureAttributes;

#define declareTintedFaceAttribute(pos, _tint) (TextureAttributes) { \
    .u = ((pos) % 16) * BLOCK_TEXTURE_SIZE, \
    .v = ((pos) / 16) * BLOCK_TEXTURE_SIZE, \
    .w = BLOCK_TEXTURE_SIZE, \
    .h = BLOCK_TEXTURE_SIZE, \
    .tint = _tint \
}

// Order
// - 0: +y DOWN
// - 1: -y UP
// - 2: -x LEFT
// - 3: +x RIGHT
// - 4: +z BACK
// - 5: -z FRONT
#define declareTintedFaceAttributes(\
    pos_y, pos_y_tint, \
    neg_y, neg_y_tint, \
    neg_x, neg_x_tint, \
    pos_x, pos_x_tint, \
    pos_z, pos_z_tint, \
    neg_z, neg_z_tint \
) \
    declareTintedFaceAttribute(pos_y, P99_PROTECT(pos_y_tint)), \
    declareTintedFaceAttribute(neg_y, P99_PROTECT(neg_y_tint)), \
    declareTintedFaceAttribute(neg_x, P99_PROTECT(neg_x_tint)), \
    declareTintedFaceAttribute(pos_x, P99_PROTECT(pos_x_tint)), \
    declareTintedFaceAttribute(pos_z, P99_PROTECT(pos_z_tint)), \
    declareTintedFaceAttribute(neg_z, P99_PROTECT(neg_z_tint))

#define faceTint(r,g,b,cd) P99_PROTECT({r,g,b,cd})
#define NO_TINT faceTint(128,128,128,0)
#define declareFaceAttributes(pos_y, neg_y, neg_x, pos_x, pos_z, neg_z) declareTintedFaceAttributes( \
    pos_y, NO_TINT, \
    neg_y, NO_TINT, \
    neg_x, NO_TINT, \
    pos_x, NO_TINT, \
    pos_z, NO_TINT, \
    neg_z, NO_TINT \
)
#define defaultFaceAttributes(index) declareFaceAttributes(index, index, index, index, index, index)

// Used when rendering a single block texture
// of size 16x16 repeating 0 or more times on
// both axes
extern const RECT single_block_texture_window;

u8 textureWindowMaskBlock(const u16 coord);
u8 textureWindowOffsetBlock(const u16 coord);

#endif // PSXMC_TEXTURE_H
