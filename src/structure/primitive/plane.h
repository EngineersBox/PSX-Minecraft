#pragma once

#ifndef _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_
#define _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_

#include <psxgte.h>

#include "../../util/inttypes.h"

typedef struct {
    VECTOR normal;
    fixedi32 distance;
} Plane;

Plane planeCreate(VECTOR p1, VECTOR normal);


#endif // _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__PLANE_H_