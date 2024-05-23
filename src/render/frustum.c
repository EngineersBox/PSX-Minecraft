#include "frustum.h"

#include <psxgte.h>
#include <logging.h>
#include <texture.h>

#include "../math/math_utils.h"

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
//     DEBUG_LOG("[FRUSTUM] Chunk AABB [Min: (%d,%d,%d)] [Max: (%d,%d,%d)]\n", inlineVec(aabb->min), inlineVec(aabb->max));
//     return testAABBPlane(aabb, &frustum->left)
//         && testAABBPlane(aabb, &frustum->right)
//         && testAABBPlane(aabb, &frustum->top)
//         && testAABBPlane(aabb, &frustum->bottom)
//         && testAABBPlane(aabb, &frustum->near)
//         && testAABBPlane(aabb, &frustum->far);
// }

FrustumQueryResult frustumTestAABBPlane(const AABB* aabb, const Plane* plane) {
    const VECTOR normal = plane->normal;
    i64 x1; i64 x2;
    i64 y1; i64 y2;
    i64 z1; i64 z2;
    if (normal.vx > 0) {
        x1 = aabb->max.vx;
        x2 = aabb->min.vx;
    } else {
        x1 = aabb->min.vx;
        x2 = aabb->max.vx;
    }
    if (normal.vy > 0) {
        y1 = aabb->max.vy;
        y2 = aabb->min.vy;
    } else {
        y1 = aabb->min.vy;
        y2 = aabb->max.vy;
    }
    if (normal.vz > 0) {
        z1 = aabb->max.vz;
        z2 = aabb->min.vz;
    } else {
        z1 = aabb->min.vz;
        z2 = aabb->max.vz;
    }
    const i64 dot_1 = fixedMul((i64) normal.vx, x1)
        + fixedMul((i64) normal.vy, y1)
        + fixedMul((i64) normal.vz, z1);
    if (dot_1 < -plane->distance) {
        DEBUG_LOG("[FRUSTUM] Dot: %d, Distance: %d\n", (i32) dot_1, (i32) plane->distance);
        return FRUSTUM_OUTSIDE;
    }
    const i64 dot_2 = fixedMul((i64) normal.vx, x2)
        + fixedMul((i64) normal.vy, y2)
        + fixedMul((i64) normal.vz, z2);
    DEBUG_LOG("[FRUSTUM] Check 2 Dot: %d, Distance: %d\n", (i32) dot_2, (i32) plane->distance);
    if (dot_2 <= -plane->distance) {
        return FRUSTUM_INTERSECTS;
    }
    return FRUSTUM_INSIDE;
}

FrustumQueryResult frustumContainsAABB(const Frustum* frustum, const AABB* aabb) {
    // DEBUG_LOG("[FRUSTUM] Chunk AABB [Min: (%d,%d,%d)] [Max: (%d,%d,%d)]\n", inlineVec(aabb->min), inlineVec(aabb->max));
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