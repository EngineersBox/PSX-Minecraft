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

Frustum frustumCreate();

void frustumTransform(Frustum* frustum, Transforms* transforms);
void frustumRestore(Frustum* frustum);

bool frustumContainsAABB(const Frustum* frustum,
                         const AABB* aabb);

#endif // _PSX_MINECRAFT__RENDER__FRUSTUM_H_