#pragma once

#ifndef PSX_MINECRAFT_CUBE_H
#define PSX_MINECRAFT_CUBE_H

#include <sys/types.h>
#include <stdint.h>
#include <psxgte.h>

#include "../resources/assets.h"
#include "../core/display.h"
#include "../render/transforms.h"
#include "../primitive/primitive.h"

#define CUBE_FACES 6

typedef struct {
    VECTOR position;
    SVECTOR rotation;
    Texture* texture;
    uint8_t texture_uvwh[CUBE_FACES][4];
    const SVECTOR* vertices;
} Cube;

extern const SVECTOR CUBE_NORMS[];
extern const INDEX CUBE_INDICES[];

void cubeRender(Cube* cube, DisplayContext* ctx, Transforms* transforms);

#endif //PSX_MINECRAFT_CUBE_H
