#pragma once

#ifndef PSXMC_CUBE_H
#define PSXMC_CUBE_H

#include <sys/types.h>
#include <stdint.h>
#include <psxgte.h>

#include "../../resources/texture.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "primitive.h"
#include "direction.h"

#define CUBE_FACES 6

typedef struct {
    VECTOR position;
    SVECTOR rotation;
    TextureAttributes texture_face_attrib[CUBE_FACES];
    size_t texture;
    const SVECTOR vertices[8];
} Cube;

extern const SVECTOR CUBE_NORMS_UNIT[FACE_DIRECTION_COUNT];
extern const SVECTOR CUBE_NORMS[FACE_DIRECTION_COUNT];
extern const INDEX CUBE_INDICES[FACE_DIRECTION_COUNT];

void cubeRender(Cube* cube, RenderContext* ctx, Transforms* transforms);

#endif //PSXMC_CUBE_H
