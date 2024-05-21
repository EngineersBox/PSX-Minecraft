#include "frustum.h"

#include <psxgte.h>
#include <logging.h>
#include <texture.h>

#include "../math/math_utils.h"

// void frustumInit(Frustum* frustum,
//                  const fixedi32 fov,
//                  const fixedi32 aspect,
//                  const fixedi32 z_near,
//                  const fixedi32 z_far) {
//     const fixedi32 fov_sin = isin(fov);
//     const fixedi32 fov_cos = icos(fov);
//     i32* verts = frustum->verts;
//     // Top
//     verts[0] = 0;
//     verts[1] = -fov_cos;
//     verts[2] = fov_sin;
//     verts[3] = 0;
//     // Bottom
//     verts[4] = 0;
//     verts[5] = fov_cos;
//     verts[6] = fov_sin;
//     verts[7] = 0;
//     // Left
//     verts[8] = fov_cos;
//     verts[9] = 0;
//     verts[10] = fixedMul(fov_sin, aspect);
//     verts[11] = 0;
//     // Right
//     verts[12] = -fov_cos;
//     verts[13] = 0;
//     verts[14] = fixedMul(fov_sin, aspect);
//     verts[15] = 0;
//     // Near
//     verts[16] = 0;
//     verts[17] = 0;
//     verts[18] = ONE;
//     verts[19] = z_far;
//     // Far
//     verts[20] = 0;
//     verts[21] = 0;
//     verts[22] = -ONE;
//     verts[23] = -z_near;
// }

// VECTOR computeFrustumPlaneNormal(VECTOR basis,
//                                  SVECTOR rotation) {
//     MATRIX m_r = {0};
//     RotMatrix(&rotation, &m_r);
//     PushMatrix();
//     VECTOR result = {0};
//     ApplyMatrixLV(&m_r, &basis, &result);
//     PopMatrix();
//     return result;
// }

#define intoVec(svec) vec3_i32(svec.vx << FIXED_POINT_SHIFT, svec.vy << FIXED_POINT_SHIFT, svec.vz << FIXED_POINT_SHIFT)

void frustumInit(Frustum* frustum,
                 const fixedi32 fov_y,
                 const fixedi32 aspect,
                 const fixedi32 z_near,
                 const fixedi32 z_far) {
// const VECTOR front = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_FRONT]);
// const VECTOR back = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_BACK]);
// const VECTOR up = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_UP]);
// const VECTOR down = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_DOWN]);
// const VECTOR right = intoVec(intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_RIGHT]));
// const VECTOR left = intoVec(intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_LEFT]));
// const fixedi32 scaled_fov_y = degToUnitRange(fov_y >> 1);
// const fixedi32 tan_fov = fixedDiv(isin(scaled_fov_y), icos(scaled_fov_y));
// DEBUG_LOG("[FRUSTUM] FOV: %d Scaled: %d Tan: %d\n", fov_y, scaled_fov_y, tan_fov);
// const fixedi32 half_v_side = z_far * tan_fov;
// const fixedi32 half_h_side = fixedMul(half_v_side, aspect);
// const fixedi32 fov_x =
// // const VECTOR back_mul_far = vector_const_mul(back, z_far);
// // const fixedi32 z_mid = (z_near + z_far) / 2;
// // Right
// const VECTOR right_normal = computeFrustumPlaneNormal(
//     right,
//     vec3_i16(0, scaled_fov, 0)
// );
// frustum->right = planeCreate(
//     // vector_const_div(vector_sub(back_mul_far, vector_const_mul(right, half_h_side)), 2),
//     vec3_i32(half_h_side >> 1, 0, z_far >> 1),
//     right_normal
// );
// // Left
// const VECTOR left_normal = computeFrustumPlaneNormal(
//     left,
//     vec3_i16(0, -scaled_fov, 0)
// );
// frustum->left = planeCreate(
//     // vector_const_div(vector_sub(back_mul_far, vector_const_mul(right, -half_h_side)), 2),
//     vec3_i32(-half_h_side >> 1, 0, z_far >> 1),
//     left_normal
// );
// const fixedi32 scaled_vert_fov = degToUnitRange(1);
// // Up
// const VECTOR up_normal = computeFrustumPlaneNormal(
//     up,
//     vec3_i16(-)
// );
    const VECTOR front = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_FRONT]);
    const VECTOR back = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_BACK]);
    const VECTOR up = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_UP]);
    const VECTOR down = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_DOWN]);
    const VECTOR right = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_RIGHT]);
    const VECTOR left = intoVec(FACE_DIRECTION_NORMALS[FACE_DIR_LEFT]);
    // Near/Far Ends
    const VECTOR near_centre = vec3_i32(0,0, z_near);
    const VECTOR far_centre = vec3_i32(0, 0, z_far);
    // Width/Height
    const fixedi32 scaled_fov = degToUnitRange(fov_y >> 1);
    const fixedi32 tan_fov = fixedDiv(isin(scaled_fov), icos(scaled_fov));
    const fixedi32 near_height = fixedMul(tan_fov, z_near) << 1;
    const fixedi32 near_width = fixedMul(near_height, aspect);
    const fixedi32 far_height = fixedMul(tan_fov, z_far) << 1;
    const fixedi32 far_width = fixedMul(far_height, aspect);
    // Far
    const VECTOR far_top_left = vec3_i32(
        far_centre.vx + fixedMul(up.vx, (far_height >> 1)) + fixedMul(left.vx, (far_width >> 1)),
        far_centre.vy + fixedMul(up.vy, (far_height >> 1)) + fixedMul(left.vy, (far_width >> 1)),
        far_centre.vz + fixedMul(up.vz, (far_height >> 1)) + fixedMul(left.vz, (far_width >> 1))
    );
    const VECTOR far_top_right = vec3_i32(
        far_centre.vx + fixedMul(up.vx, (far_height >> 1)) + fixedMul(right.vx, (far_width >> 1)),
        far_centre.vy + fixedMul(up.vy, (far_height >> 1)) + fixedMul(right.vy, (far_width >> 1)),
        far_centre.vz + fixedMul(up.vz, (far_height >> 1)) + fixedMul(right.vz, (far_width >> 1))
    );
    const VECTOR far_bottom_left = vec3_i32(
        far_centre.vx + fixedMul(down.vx, (far_height >> 1)) + fixedMul(left.vx, (far_width >> 1)),
        far_centre.vy + fixedMul(down.vy, (far_height >> 1)) + fixedMul(left.vy, (far_width >> 1)),
        far_centre.vz + fixedMul(down.vz, (far_height >> 1)) + fixedMul(left.vz, (far_width >> 1))
    );
    const VECTOR far_bottom_right = vec3_i32(
        far_centre.vx + fixedMul(down.vx, (far_height >> 1)) + fixedMul(right.vx, (far_width >> 1)),
        far_centre.vy + fixedMul(down.vy, (far_height >> 1)) + fixedMul(right.vy, (far_width >> 1)),
        far_centre.vz + fixedMul(down.vz, (far_height >> 1)) + fixedMul(right.vz, (far_width >> 1))
    );
    // Near
    const VECTOR near_top_left = vec3_i32(
        near_centre.vx + fixedMul(up.vx, (near_height >> 1)) + fixedMul(left.vx, (near_width >> 1)),
        near_centre.vy + fixedMul(up.vy, (near_height >> 1)) + fixedMul(left.vy, (near_width >> 1)),
        near_centre.vz + fixedMul(up.vz, (near_height >> 1)) + fixedMul(left.vz, (near_width >> 1))
    );
    const VECTOR near_top_right = vec3_i32(
        near_centre.vx + fixedMul(up.vx, (near_height >> 1)) + fixedMul(right.vx, (near_width >> 1)),
        near_centre.vy + fixedMul(up.vy, (near_height >> 1)) + fixedMul(right.vy, (near_width >> 1)),
        near_centre.vz + fixedMul(up.vz, (near_height >> 1)) + fixedMul(right.vz, (near_width >> 1))
    );
    const VECTOR near_bottom_left = vec3_i32(
        near_centre.vx + fixedMul(down.vx, (near_height >> 1)) + fixedMul(left.vx, (near_width >> 1)),
        near_centre.vy + fixedMul(down.vy, (near_height >> 1)) + fixedMul(left.vy, (near_width >> 1)),
        near_centre.vz + fixedMul(down.vz, (near_height >> 1)) + fixedMul(left.vz, (near_width >> 1))
    );
    const VECTOR near_bottom_right = vec3_i32(
        near_centre.vx + fixedMul(down.vx, (near_height >> 1)) + fixedMul(right.vx, (near_width >> 1)),
        near_centre.vy + fixedMul(down.vy, (near_height >> 1)) + fixedMul(right.vy, (near_width >> 1)),
        near_centre.vz + fixedMul(down.vz, (near_height >> 1)) + fixedMul(right.vz, (near_width >> 1))
    );
    // Planes
    // Near
    frustum->near = planeCreate(
        near_centre,
        front
    );
    // Far
    frustum->far = planeCreate(
        far_centre,
        back
    );
    VECTOR p0, p1, p2;
    // Left
    p0 = near_bottom_left; p1 = far_bottom_left; p2 = far_top_left;
    frustum->left = planeCreate(
        p0,
        vec3_i32_normalize(_cross(
            vector_sub(p1, p0),
            vector_sub(p2, p1)
        ))
    );
    // Top
    p0 = near_top_left; p1 = far_top_left; p2 = far_top_right;
    frustum->top = planeCreate(
        p0,
        vec3_i32_normalize(_cross(
            vector_sub(p1, p0),
            vector_sub(p2, p1)
        ))
    );
    // Right
    p0 = near_top_right; p1 = far_top_right; p2 = far_bottom_right;
    frustum->top = planeCreate(
        p0,
        vec3_i32_normalize(_cross(
            vector_sub(p1, p0),
            vector_sub(p2, p1)
        ))
    );
    // Bottom
    p0 = near_bottom_right; p1 = far_bottom_right; p2 = far_bottom_left;
    frustum->top = planeCreate(
        p0,
        vec3_i32_normalize(_cross(
            vector_sub(p1, p0),
            vector_sub(p2, p1)
        ))
    );
}

#undef intoVec

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

bool testAABBPlane(const AABB* aabb, const Plane* plane) {
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
    const i64 dot_1 = fixedMul(normal.vx, x1)
        + fixedMul(normal.vy, y1)
        + fixedMul(normal.vz, z1);
    if (dot_1 < plane->distance) {
        return false; // Outside
    }
    const i64 dot_2 = fixedMul(normal.vx, x2)
        + fixedMul(normal.vy, y2)
        + fixedMul(normal.vz, z2);
    return dot_2 <= plane->distance;
}

bool frustumContainsAABB(const Frustum* frustum,
                          const AABB* aabb) {
    DEBUG_LOG("[FRUSTUM] Chunk AABB [Min: (%d,%d,%d)] [Max: (%d,%d,%d)]\n", inlineVec(aabb->min), inlineVec(aabb->max));
    return testAABBPlane(aabb, &frustum->left)
        && testAABBPlane(aabb, &frustum->right)
        && testAABBPlane(aabb, &frustum->top)
        && testAABBPlane(aabb, &frustum->bottom)
        && testAABBPlane(aabb, &frustum->near)
        && testAABBPlane(aabb, &frustum->far);
}