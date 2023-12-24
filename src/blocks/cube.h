#pragma once

#ifndef PSX_MINECRAFT_CUBE_H
#define PSX_MINECRAFT_CUBE_H

#include <psxgte.h>

#include "../core/display.h"
#include "../primitive/primitive.h"

typedef struct Cube {
    VECTOR psoition;
    VECTOR rotation;
    uint16_t texture_tpage;
    uint16_t texture_clut;
    SVECTOR vertices[8];
} Cube;

extern const SVECTOR CUBE_NORMS[];
extern const INDEX CUBE_INDICES[];

#define CUBE_FACES 6

void cubeRender(DisplayContext* ctx, Cube* cube);

#endif //PSX_MINECRAFT_CUBE_H
