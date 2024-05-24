#include "frustum.h"

#include <psxgte.h>

#include "../logging/logging.h"
#include "../math/math_utils.h"
#include "../util/inttypes.h"

Frustum frustumCreate() {
    // Pre-calculated with frustum_calculator.py
    return (Frustum) {
        .planes = {
            [FRUSTUM_PLANE_NEAR] = (Plane) { .normal = vec3_i32(0, 0, -4096), .distance = 409 },
            [FRUSTUM_PLANE_FAR] = (Plane) { .normal = vec3_i32(0, 0, 4096), .distance = 4096000 },
            [FRUSTUM_PLANE_LEFT] = (Plane) { .normal = vec3_i32(-2457, 0, 3276), .distance = 0 },
            [FRUSTUM_PLANE_RIGHT] = (Plane) { .normal = vec3_i32(2457, 0, 3276), .distance = 0 },
            [FRUSTUM_PLANE_TOP] = (Plane) { .normal = vec3_i32(0, -2896, 2896), .distance = 0 },
            [FRUSTUM_PLANE_BOTTOM] = (Plane) { .normal = vec3_i32(0, 2896, 2896), .distance = 0 },
        }
    };
}

// bool testAABBPlane(const AABB* aabb,
//                    const Plane* plane) {
//     // Convert AABB to centre-extents representation
//     const VECTOR c = vec3_i32(
//         (aabb->max.vx + aabb->min.vx) >> 1,
//         (aabb->max.vy + aabb->min.vy) >> 1,
//         (aabb->max.vz + aabb->min.vz) >> 1
//     );
//     // Compute positive extents
//     const VECTOR e = vector_sub(aabb->max, c);
//     // Compute the projection interval radius of b onto
//     // L(t) = aabb->centre + t * plane->normal
//     const fixedi32 r = fixedMul(e.vx, absv(plane->normal.vx))
//         + fixedMul(e.vy, absv(plane->normal.vy))
//         + fixedMul(e.vz, absv(plane->normal.vz));
//     // Compute distance of box centre from plane
//     const fixedi32 s = dot(&plane->normal, &c) - plane->distance;
//     // Intersection ocurs when distance s falls within [-r,+r] interval
//     return absv(s) <= r;
// }
//
// bool frustumContainsAABB(const Frustum* frustum,
//                          const AABB* aabb) {
//     DEBUG_LOG("[FRUSTUM] Chunk AABB [Min: " VEC_PATTERN "] [Max: " VEC_PATTERN "]\n", VEC_LAYOUT(aabb->min), VEC_LAYOUT(aabb->max));
//     return testAABBPlane(aabb, &frustum->left)
//         && testAABBPlane(aabb, &frustum->right)
//         && testAABBPlane(aabb, &frustum->top)
//         && testAABBPlane(aabb, &frustum->bottom)
//         && testAABBPlane(aabb, &frustum->near)
//         && testAABBPlane(aabb, &frustum->far);
// }

FrustumQueryResult frustumTestAABBPlane(const AABB* aabb, const Plane* plane) {
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
    if (dot_1 < -plane->distance) {
        DEBUG_LOG("[FRUSTUM] Dot: %d, Distance: %d\n", (i32) dot_1, (i32) plane->distance);
        return FRUSTUM_OUTSIDE;
    }
    const i64 dot_2 = dot_i64(normal, vec2);
    DEBUG_LOG("[FRUSTUM] Check 2 Dot: %d, Distance: %d\n", (i32) dot_2, (i32) plane->distance);
    if (dot_2 <= -plane->distance) {
        return FRUSTUM_INTERSECTS;
    }
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