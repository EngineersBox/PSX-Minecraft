#include "frustum.h"

#include <psxgte.h>
#include <inline_c.h>

#include "../logging/logging.h"
#include "../math/math_utils.h"
#include "../math/vector.h"
#include "../util/inttypes.h"
#include "../render/duration_tree.h"

// TODO: Limit to a single plane facing directly forward and verify transformation
//       of position and rotation of normal (use right , i.e. (1,0,0)). This ensures
//       the calculations are correct. We should see chunks in the direction of the
//       normal being culled.
Frustum frustumCreate() {
    // Pre-calculated with frustum_calculator.py
    return (Frustum) {
        .planes = {
            /*[FRUSTUM_PLANE_NEAR] = (Plane) {*/
            /*    .normal = vec3_i32(4096, 0, 0),*/
            /*    .point = vec3_i32(0),*/
            /*    .distance = 0*/
            /*},*/
            [FRUSTUM_PLANE_NEAR] = (Plane) { .normal = vec3_i32(0, 0, -4096), .distance = 1/*409*/, .point = vec3_i32(0, 0, 1/*409*/) },
            [FRUSTUM_PLANE_FAR] = (Plane) { .normal = vec3_i32(0, 0, 4096), .distance = 4096, .point = vec3_i32(0, 0, 4096) },
            [FRUSTUM_PLANE_LEFT] = (Plane) { .normal = vec3_i32(-2457, 0, 3276), .distance = 0, .point = vec3_i32(0) },
            [FRUSTUM_PLANE_RIGHT] = (Plane) { .normal = vec3_i32(2457, 0, 3276), .distance = 0, .point = vec3_i32(0) },
            [FRUSTUM_PLANE_TOP] = (Plane) { .normal = vec3_i32(0, -2896, 2896), .distance = 0, .point = vec3_i32(0) },
            [FRUSTUM_PLANE_BOTTOM] = (Plane) { .normal = vec3_i32(0, 2896, 2896), .distance = 0, .point = vec3_i32(0) },
        }
    };
}

DEFN_DURATION_COMPONENT(frustum_render);

static Plane current_planes[6] = {0};

// TODO: Only transform/restore when the camera has moved, otherwise keep reusing the current planes
void frustumTransform(Frustum* frustum, Transforms* transforms) {
    durationComponentInitOnce(frustum_render, "frustumTransform");
    durationComponentStart(&frustum_render_duration);
    // NOTE: Plane normals should be rotated without translation vector 
    //       applied to geometry matrix. The reason is that we never
    //       transform normals with homogeneous coordinates. See This
    //       SO post for details: https://stackoverflow.com/a/10597767
    MATRIX* rot_mat = {0};
    InvRotMatrix(&transforms->negative_translation_rotation, rot_mat);
    for (u8 i = 0; i < 6; i++) {
        Plane* plane = &frustum->planes[i];
        /*DEBUG_LOG(*/
        /*    "[FRUSTUM :: PLANE %d] Normal: " VEC_PATTERN " Point: " VEC_PATTERN " Dot: " INT64_PATTERN "\n",*/
        /*    i,*/
        /*    VEC_LAYOUT(plane->normal),*/
        /*    VEC_LAYOUT(plane->point),*/
        /*    INT64_LAYOUT(plane->distance)*/
        /*);*/
        current_planes[i] = *plane;
        ApplyMatrixLV(
            rot_mat,
            &plane->normal,
            &plane->normal
        );
        plane->normal = vec3_i32_normalize(plane->normal);
        plane->point = applyMatrixRotTrans(
            transforms->frustum_mtx,
            plane->point
        );
        plane->distance = dot_i64(plane->normal, plane->point);
        /*DEBUG_LOG(*/
        /*    "[FRUSTUM :: PLANE %d] Normal: " VEC_PATTERN " Point: " VEC_PATTERN " Dot: " INT64_PATTERN "\n",*/
        /*    i,*/
        /*    VEC_LAYOUT(plane->normal),*/
        /*    VEC_LAYOUT(plane->point),*/
        /*    INT64_LAYOUT(plane->distance)*/
        /*);*/
    }
    durationComponentEnd();
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
        vec1.v##axis = aabb->max.v##axis; \
        vec2.v##axis = aabb->min.v##axis; \
    } else { \
        vec1.v##axis = aabb->min.v##axis; \
        vec2.v##axis = aabb->max.v##axis; \
    }
    pickBoundsFromSign(x);
    pickBoundsFromSign(y);
    pickBoundsFromSign(z);
#undef pickBoundsFromSign
    /*DEBUG_LOG(*/
    /*    "[FRUSTUM] Check 1 Normal: " VEC_PATTERN " Vec2: " VEC_PATTERN "\n",*/
    /*    VEC_LAYOUT(normal),*/
    /*    VEC_LAYOUT(vec1)*/
    /*);*/
    const i64 dot_1 = dot_i64(
        normal,
        vec1 
    ) + plane->distance;
    /*DEBUG_LOG(*/
    /*    "[FRUSTUM] Check 1 Dot: " INT64_PATTERN ", Distance: " INT64_PATTERN "\n",*/
    /*    INT64_LAYOUT(dot_1),*/
    /*    INT64_LAYOUT(plane->distance)*/
    /*);*/
    if (dot_1 < 0) {
        // AABB max point is outside the frustum
        return FRUSTUM_OUTSIDE;
    }
    // AABB max point is inside the frustum
    /*DEBUG_LOG(*/
    /*    "[FRUSTUM] Check 2 Normal: " VEC_PATTERN " Vec2: " VEC_PATTERN "\n",*/
    /*    VEC_LAYOUT(normal),*/
    /*    VEC_LAYOUT(vec2)*/
    /*);*/
    const i64 dot_2 = dot_i64(
        normal,
        vec2 
    ) + plane->distance;
    /*DEBUG_LOG(*/
    /*    "[FRUSTUM] Check 2 Dot: " INT64_PATTERN ", Distance: " INT64_PATTERN "\n",*/
    /*    INT64_LAYOUT(dot_2),*/
    /*    INT64_LAYOUT(plane->distance)*/
    /*);*/
    if (dot_2 <= 0) {
        // AABB min point is outside or on the edge of the frustum
        return FRUSTUM_INTERSECTS;
    }
    // AABB min point is inside the frustum
    return FRUSTUM_INSIDE;
}

FrustumQueryResult frustumContainsAABB(const Frustum* frustum, const AABB* aabb) {
    // DEBUG_LOG("[FRUSTUM] Chunk AABB [Min: " VEC_PATTERN "] [Max: " VEC_PATTERN "]\n", VEC_LAYOUT(aabb->min), VEC_LAYOUT(aabb->max));
    FrustumQueryResult result = FRUSTUM_INSIDE;
    // NOTE: Skip near/far culling as we do that in chunk traversal
    //       during worldRender as a bounded traversal depth
    for (u8 i = 2; i < 6; i++) {
        switch (frustumTestAABBPlane(aabb, &frustum->planes[FRUSTUM_PLANE_NEAR])) {
            case FRUSTUM_OUTSIDE: return FRUSTUM_OUTSIDE;
            case FRUSTUM_INTERSECTS: result = FRUSTUM_INTERSECTS; break;
            case FRUSTUM_INSIDE: break;
        }
    }
    return result;
}
