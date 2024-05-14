#pragma once

#ifndef PSX_MINECRAFT_TEXTURE_H
#define PSX_MINECRAFT_TEXTURE_H

#include <psxgte.h>
#include <stdint.h>

typedef enum {
    FACE_DIR_DOWN = 0,
    FACE_DIR_UP,
    FACE_DIR_LEFT,
    FACE_DIR_RIGHT,
    FACE_DIR_BACK,
    FACE_DIR_FRONT
} FaceDirection;

typedef struct {
    uint16_t tpage;
    uint16_t clut;
} Texture;

typedef struct {
    uint8_t u;
    uint8_t v;
    uint8_t w;
    uint8_t h;
    CVECTOR tint;
} TextureAttributes;

#define declareTintedFaceAttribute(pos, tint) { \
    ((pos) % 16) * BLOCK_TEXTURE_SIZE, \
    ((pos) / 16) * BLOCK_TEXTURE_SIZE, \
    BLOCK_TEXTURE_SIZE, \
    BLOCK_TEXTURE_SIZE, \
    tint \
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
) { \
    declareTintedFaceAttribute(pos_y, P99_PROTECT(pos_y_tint)), \
    declareTintedFaceAttribute(neg_y, P99_PROTECT(neg_y_tint)), \
    declareTintedFaceAttribute(neg_x, P99_PROTECT(neg_x_tint)), \
    declareTintedFaceAttribute(pos_x, P99_PROTECT(pos_x_tint)), \
    declareTintedFaceAttribute(pos_z, P99_PROTECT(pos_z_tint)), \
    declareTintedFaceAttribute(neg_z, P99_PROTECT(neg_z_tint)) \
}
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

#endif // PSX_MINECRAFT_TEXTURE_H
