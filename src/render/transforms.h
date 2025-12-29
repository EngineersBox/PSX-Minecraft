#pragma once

#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include <psxgte.h>

typedef struct {
    VECTOR translation_position;
    // VECTOR negative_translation_position;
    SVECTOR	translation_rotation;
    // SVECTOR negative_translation_rotation;
    MATRIX geometry_mtx;
    // MATRIX frustum_mtx;
    MATRIX lighting_mtx;
} Transforms;

#endif //TRANSFORMS_H
