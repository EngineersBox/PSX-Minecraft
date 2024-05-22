#pragma once

#ifndef _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_
#define _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_

#include <psxgte.h>
#include <stdbool.h>

#include "../../render/transforms.h"
#include "../../util/inttypes.h"

typedef struct {
    VECTOR normal;
    i64 distance;
} Plane;

Plane planeCreate(const VECTOR point, const VECTOR normal);
bool planePointInFront(const Plane* plane, const VECTOR point);

#endif // _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_