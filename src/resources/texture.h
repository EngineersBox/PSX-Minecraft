#pragma once

#ifndef PSXMC_TEXTURE_H
#define PSXMC_TEXTURE_H

#include <psxgte.h>
#include <psxgpu.h>

#include "../util/inttypes.h"
#include "../math/vector.h"
#include "../util/preprocessor.h"

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

/* Texture window generation, courtesy of @kenchup_ on the PSX.Dev discord
tilePrimX = {8: 0b11111, 16: 0b11110, 32: 0b11100, 64: 0b11000, 128: 0b10000, 256: 0b00000}[mat.tileX]
tilePrimY = {8: 0b11111, 16: 0b11110, 32: 0b11100, 64: 0b11000, 128: 0b10000, 256: 0b00000}[mat.tileY]
tpOffsetX = mat.xPos & 0b111111
tpOffsetX = tpOffsetX << {15: 0, 8: 1, 4: 2}[mat.colorMode]
tpOffsetY = mat.yPos & 0xFF
tilePrimOffsetX = tpOffsetX>>3
tilePrimOffsetY = tpOffsetY>>3
tilePrim = (0xE20<<20) + (tilePrimOffsetY<<15) + (tilePrimOffsetX<<10) + (tilePrimY<<5) + (tilePrimX)
 */
typedef RECT TextureWindow ;

#define textureWindowOffset(coord) (((coord) >> 3) & 0b11111)

#define TEXTURE_WINDOW_MASK_8 0b11111
#define TEXTURE_WINDOW_MASK_16 0b11110
#define TEXTURE_WINDOW_MASK_32 0b11100
#define TEXTURE_WINDOW_MASK_64 0b11000
#define TEXTURE_WINDOW_MASK_128 0b10000
#define TEXTURE_WINDOW_MASK_256 0b00000

#define textureWindowCreateDirect(tile_x, tile_y, u, v) ((TextureWindow) {\
    .w = tile_x, \
    .h = tile_y, \
    .x = textureWindowOffset(u), \
    .y = textureWindowOffset(v), \
})

#define textureWindowCreate(tile_x, tile_y, u, v) textureWindowCreateDirect( \
    TEXTURE_WINDOW_MASK_##tile_x, \
    TEXTURE_WINDOW_MASK_##tile_y, \
    textureWindowOffset(u), \
    textureWindowOffset(v) \
)

#endif // PSXMC_TEXTURE_H
