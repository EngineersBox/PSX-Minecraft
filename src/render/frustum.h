#pragma once

#ifndef _PSX_MINECRAFT__RENDER__FRUSTUM_H_
#define _PSX_MINECRAFT__RENDER__FRUSTUM_H_

#include <psxgte.h>

#include "../util/inttypes.h"
#include "../physics/aabb.h"
#include "../structure/primitive/plane.h"
#include "transforms.h"

typedef struct {
    Plane top;
    Plane bottom;
    Plane left;
    Plane right;
    Plane far;
    Plane near;
} Frustum;

// fov should be in the range [0,4096]. To convert degrees to
// this, take x (in degrees) and apply: (x / 360) * 4096
void frustumInit(Frustum* frustum,
                 VECTOR front,
                 VECTOR right,
                 VECTOR up,
                 fixedi32 fov,
                 fixedi32 aspect,
                 fixedi32 z_near,
                 fixedi32 z_far);

bool frustumContainsAABB(const Frustum* frustum, const AABB* aabb);

#endif // _PSX_MINECRAFT__RENDER__FRUSTUM_H_