#pragma once

#ifndef _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_
#define _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_

#include <psxgte.h>

#include "../../render/transforms.h"
#include "../../util/inttypes.h"

typedef struct {
    VECTOR normal;
    i64 distance;
} Plane;

Plane planeCreate(VECTOR point, VECTOR normal);

#endif // _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_