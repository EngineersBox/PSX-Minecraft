#include "frustum.h"

#include <math_utils.h>

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

void frustumInit(Frustum* frustum,
                 const VECTOR front,
                 const VECTOR right,
                 const VECTOR up,
                 const fixedi32 fov,
                 const fixedi32 aspect,
                 const fixedi32 z_near,
                 const fixedi32 z_far) {
    const fixedi32 half_v_side = z_far * itan(fov >> 1);
    const fixedi32 half_h_side = fixedMul(half_v_side, aspect);
    const VECTOR front_mul_far = vector_const_mul(front, z_far);
    frustum->near = planeCreate(
        vector_const_mul(front, z_near),
        front
    );
    frustum->far = planeCreate(
        front_mul_far,
        vector_const_mul(front, -1)
    );
    const VECTOR right_half_h_side = vec3_i32(
        fixedMul(right.vx, half_h_side),
        fixedMul(right.vy, half_h_side),
        fixedMul(right.vz, half_h_side)
    );
    frustum->right = planeCreate(
        vec3_i32_all(0),
        cross(
            &vector_sub(
                front_mul_far,
                right_half_h_side
            ),
            &up
        )
    );
    frustum->left = planeCreate(
        vec3_i32_all(0),
        cross(
            &up,
            &vector_add(
                front_mul_far,
                right_half_h_side
            )
        )
    );
    const VECTOR up_half_v_side = vec3_i32(
        fixedMul(up.vx, half_v_side),
        fixedMul(up.vy, half_v_side),
        fixedMul(up.vz, half_v_side)
    );
    frustum->top = planeCreate(
        vec3_i32_all(0),
        cross(
            &right,
            &vector_sub(
                front_mul_far,
                up_half_v_side
            )
        )
    );
    frustum->bottom = planeCreate(
        vec3_i32_all(0),
        cross(
            &vector_add(
                front_mul_far,
                up_half_v_side
            ),
            &right
        )
    );
}

bool frustumContainsAABB(const Frustum* frustum,
                         const AABB* aabb,
                         const Transforms* transforms) {
    // TODO: Implement this
    return true;
}