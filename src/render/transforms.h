#pragma once

#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include <psxgte.h>

typedef struct {
    VECTOR translation_position;
    SVECTOR	translation_rotation;
    MATRIX geometry_mtx;
    MATRIX lighting_mtx;
} Transforms;

#endif //TRANSFORMS_H
