#pragma once

#ifndef _PSX_MINECRAFT__RENDER__FRUSTUM_H_
#define _PSX_MINECRAFT__RENDER__FRUSTUM_H_

#include <psxgte.h>

#include "../util/inttypes.h"
#include "../physics/aabb.h"
#include "../structure/primitive/plane.h"
#include "transforms.h"

typedef enum {
    FRUSTUM_PLANE_NEAR,
    FRUSTUM_PLANE_FAR,
    FRUSTUM_PLANE_LEFT,
    FRUSTUM_PLANE_RIGHT,
    FRUSTUM_PLANE_TOP,
    FRUSTUM_PLANE_BOTTOM,
} FrustumPlanes;

typedef struct {
    Plane planes[6];
} Frustum;

typedef enum {
    FRUSTUM_INSIDE = 0,
    FRUSTUM_OUTSIDE,
    FRUSTUM_INTERSECTS
} FrustumQueryResult;

Frustum frustumCreate();

void frustumTransform(Frustum* frustum, Transforms* transforms);
void frustumRestore(Frustum* frustum);

FrustumQueryResult frustumContainsAABB(const Frustum* frustum, const AABB* aabb);

#endif // _PSX_MINECRAFT__RENDER__FRUSTUM_H_