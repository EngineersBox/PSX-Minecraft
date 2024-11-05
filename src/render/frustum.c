#include "frustum.h"

#include <psxgte.h>
#include <inline_c.h>

#include "../logging/logging.h"
#include "../math/math_utils.h"
#include "../math/vector.h"
#include "../util/inttypes.h"

Frustum frustumCreate() {
    // Pre-calculated with frustum_calculator.py
    return (Frustum) {
        .planes = {
            [FRUSTUM_PLANE_NEAR] = (Plane) { .normal = vec3_i32(0, 0, -4096), .distance = 1/*409*/, .point = vec3_i32(0, 0, 1/*409*/) },
            [FRUSTUM_PLANE_FAR] = (Plane) { .normal = vec3_i32(0, 0, 4096), .distance = 5000/*4096000*/, .point = vec3_i32(0, 0, 5000/*4096000*/) },
            [FRUSTUM_PLANE_LEFT] = (Plane) { .normal = vec3_i32(-2457, 0, 3276), .distance = 0, .point = vec3_i32_all(0) },
            [FRUSTUM_PLANE_RIGHT] = (Plane) { .normal = vec3_i32(2457, 0, 3276), .distance = 0, .point = vec3_i32_all(0) },
            [FRUSTUM_PLANE_TOP] = (Plane) { .normal = vec3_i32(0, -2896, 2896), .distance = 0, .point = vec3_i32_all(0) },
            [FRUSTUM_PLANE_BOTTOM] = (Plane) { .normal = vec3_i32(0, 2896, 2896), .distance = 0, .point = vec3_i32_all(0) },
        }
    };
}

Plane current_planes[6] = {0};

// TODO: Only transform/restore when the camera has moved, otherwise keep reusing the current planes
void frustumTransform(Frustum* frustum, Transforms* transforms) {
    for (u8 i = 0; i < 6; i++) {
        Plane* plane = &frustum->planes[i];
        DEBUG_LOG(
            "[FRUSTUM :: PLANE %d] Normal: " VEC_PATTERN " Point: " VEC_PATTERN " Dot: " INT64_PATTERN "\n",
            i,
            VEC_LAYOUT(plane->normal),
            VEC_LAYOUT(plane->point),
            INT64_LAYOUT(plane->distance)
        );
        current_planes[i] = *plane;
        ApplyMatrixLV(
            &transforms->frustum_mtx,
            &plane->normal,
            &plane->normal
        );
        /*plane->normal = vec3_i32_normalize(plane->normal);*/
        // FIXME: This doesn't transform the point properly for some reason.
        plane->point = applyGeometryMatrix(
            transforms->frustum_mtx,
            plane->point
        );
        plane->distance = dot_i64(plane->normal, plane->point);
        DEBUG_LOG(
            "[FRUSTUM :: PLANE %d] Normal: " VEC_PATTERN " Point: " VEC_PATTERN " Dot: " INT64_PATTERN "\n",
            i,
            VEC_LAYOUT(plane->normal),
            VEC_LAYOUT(plane->point),
            INT64_LAYOUT(plane->distance)
        );
    }
}

void frustumRestore(Frustum* frustum) {
    #pragma GCC unroll 6
    for (u8 i = 0; i < 6; i++) {
        frustum->planes[i] = current_planes[i];
    }
}

/*
 * NOTE: For dot products with plane normals, positive implies behind the
 *       plane, and negative implies in front of the plane.
 */

static FrustumQueryResult frustumTestAABBPlane(const AABB* aabb, const Plane* plane) {
    const VECTOR normal = plane->normal;
    LVECTOR vec1;
    LVECTOR vec2;
#define pickBoundsFromSign(axis) \
    if (normal.v##axis > 0) { \
        vec1.v##axis = aabb->max.v##axis; vec2.v##axis = aabb->min.v##axis; \
    } else { \
        vec1.v##axis = aabb->min.v##axis; vec2.v##axis = aabb->max.v##axis; \
    }
    pickBoundsFromSign(x);
    pickBoundsFromSign(y);
    pickBoundsFromSign(z);
#undef pickBoundsFromSign
    const i64 dot_1 = dot_i64(normal, vec1);
    if (dot_1 > plane->distance) {
        // AABB max point is outside the frustum
        // DEBUG_LOG("[FRUSTUM] Dot: " INT64_PATTERN ", Distance: %d\n", INT64_LAYOUT(dot_1), (i32) plane->distance);
        return FRUSTUM_OUTSIDE;
    }
    // AABB max point is inside the frustum
    const i64 dot_2 = dot_i64(normal, vec2);
    // DEBUG_LOG("[FRUSTUM] Check 2 Dot: " INT64_PATTERN ", Distance: %d\n", INT64_LAYOUT(dot_2), (i32) plane->distance);
    if (dot_2 >= plane->distance) {
        // AABB min point is outside or on the edge of the frustum
        return FRUSTUM_INTERSECTS;
    }
    // AABB min point is inside the frustum
    return FRUSTUM_INSIDE;
}

FrustumQueryResult frustumContainsAABB(const Frustum* frustum, const AABB* aabb) {
    // DEBUG_LOG("[FRUSTUM] Chunk AABB [Min: " VEC_PATTERN "] [Max: " VEC_PATTERN "]\n", VEC_LAYOUT(aabb->min), VEC_LAYOUT(aabb->max));
    FrustumQueryResult result = FRUSTUM_INSIDE;
    #pragma GCC unroll 6
    for (u8 i = 0; i < 6; i++) {
        switch (frustumTestAABBPlane(aabb, &frustum->planes[i])) {
            case FRUSTUM_OUTSIDE: return FRUSTUM_OUTSIDE;
            case FRUSTUM_INTERSECTS: result = FRUSTUM_INTERSECTS; break;
            case FRUSTUM_INSIDE: break;
        }
    }
    return result;
}
