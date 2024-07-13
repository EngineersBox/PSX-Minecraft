#pragma once

#ifndef _PSXMC__STRUCTURE_PRIMITIVE__PLANE_H_
#define _PSXMC__STRUCTURE_PRIMITIVE__PLANE_H_

#include <psxgte.h>
#include <stdbool.h>

#include "../../util/inttypes.h"

typedef struct {
    VECTOR normal;
    VECTOR point;
    i64 distance;
} Plane;

bool planePointInFront(const Plane* plane, const VECTOR point);

#endif // _PSXMC__STRUCTURE_PRIMITIVE__PLANE_H_