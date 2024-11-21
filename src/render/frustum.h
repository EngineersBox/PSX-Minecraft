#pragma once

#ifndef _PSXMC__RENDER__FRUSTUM_H_
#define _PSXMC__RENDER__FRUSTUM_H_

#include "../physics/aabb.h"
#include "../structure/primitive/plane.h"
#include "transforms.h"
#include "../util/preprocessor.h"

#define FRUSTUM_PLANES_LIST(f) \
    f(ENUM_ENTRY(FRUSTUM_PLANE_NEAR)), \
    f(ENUM_ENTRY(FRUSTUM_PLANE_FAR)), \
    f(ENUM_ENTRY(FRUSTUM_PLANE_LEFT)), \
    f(ENUM_ENTRY(FRUSTUM_PLANE_RIGHT)), \
    f(ENUM_ENTRY(FRUSTUM_PLANE_TOP)), \
    f(ENUM_ENTRY(FRUSTUM_PLANE_BOTTOM))

typedef enum {
    FRUSTUM_PLANES_LIST(enumConstruct)
} FrustumPlanes;

typedef struct {
    Plane planes[6];
} Frustum;

// Ordinal values are important as this allows us
// to treat the query result as a regular C bool,
// where 0 is false and anything else is true
#define FRUSTUM_QUERY_RESULT_LIST(f) \
    f(ENUM_ENTRY_ORD(FRUSTUM_OUTSIDE, 0)), \
    f(ENUM_ENTRY_ORD(FRUSTUM_INSIDE, 1)), \
    f(ENUM_ENTRY_ORD(FRUSTUM_INTERSECTS, 2)),

typedef enum {
    FRUSTUM_QUERY_RESULT_LIST(enumConstruct)
} FrustumQueryResult;

Frustum frustumCreate();

void frustumTransform(Frustum* frustum, Transforms* transforms);
void frustumRestore(Frustum* frustum);

FrustumQueryResult frustumContainsAABB(const Frustum* frustum, const AABB* aabb);

#endif // _PSXMC__RENDER__FRUSTUM_H_
