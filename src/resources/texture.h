#pragma once

#ifndef PSX_MINECRAFT_TEXTURE_H
#define PSX_MINECRAFT_TEXTURE_H

#include <stdint.h>

#include "colour.h"

typedef struct {
    uint16_t tpage;
    uint16_t clut;
} Texture;

typedef struct {
    uint8_t u;
    uint8_t v;
    uint8_t w;
    uint8_t h;
    RGB tint;
} TextureAttributes;

#endif // PSX_MINECRAFT_TEXTURE_H
