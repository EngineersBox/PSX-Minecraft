#include "camera.h"

#include <psxapi.h>
#include <psxgte.h>

#include "../logging/logging.h"
#include "../blocks/block.h"
#include "../math/vector.h"

Camera cameraCreate(Transforms* transforms) {
    return (Camera) {
        .transforms = transforms,
        .frustum = frustumCreate(),
        .position = vec3_i32_all(0),
        .rotation = vec3_i32_all(0),
        .mode = 0
    };
}

void lookAt(const VECTOR* eye, const VECTOR* at, const SVECTOR* up, MATRIX* mtx) {
    VECTOR taxis = vec3_sub(*at, *eye);
    SVECTOR zaxis;
    SVECTOR xaxis;
    SVECTOR yaxis;
    VectorNormalS(&taxis, &zaxis);
    taxis = cross_i32(zaxis, *up);
    VectorNormalS(&taxis, &xaxis);
    taxis = cross_i32(zaxis, xaxis);
    VectorNormalS(&taxis, &yaxis);
    mtx->m[0][0] = xaxis.vx;
    mtx->m[1][0] = yaxis.vx;
    mtx->m[2][0] = zaxis.vx;
    mtx->m[0][1] = xaxis.vy;
    mtx->m[1][1] = yaxis.vy;
    mtx->m[2][1] = zaxis.vy;
    mtx->m[0][2] = xaxis.vz;
    mtx->m[1][2] = yaxis.vz;
    mtx->m[2][2] = zaxis.vz;
    VECTOR pos = {
        .vx = -eye->vx,
        .vy = -eye->vy,
        .vz = -eye->vz
    };
    VECTOR vec;
    ApplyMatrixLV(mtx, &pos, &vec);
    TransMatrix(mtx, &vec);
}

void cameraUpdate(Camera* camera) {
    Transforms* transforms = camera->transforms;
    // Divide out fractions of camera rotation
    transforms->translation_rotation = vec3_i16(
        camera->rotation.vx >> FIXED_POINT_SHIFT,
        camera->rotation.vy >> FIXED_POINT_SHIFT,
        camera->rotation.vz >> FIXED_POINT_SHIFT
    );
    transforms->negative_translation_rotation = vec3_i16(
        ONE - (camera->rotation.vx >> FIXED_POINT_SHIFT),
        ONE - (camera->rotation.vy >> FIXED_POINT_SHIFT),
        ONE - (camera->rotation.vz >> FIXED_POINT_SHIFT)
    );
    DEBUG_LOG(
        "[CAMERA] Rot: " VEC_PATTERN " InvRot: " VEC_PATTERN "\n",
        VEC_LAYOUT(transforms->translation_rotation),
        VEC_LAYOUT(transforms->negative_translation_rotation)
    );
    // First-person camera mode
    if (camera->mode == 0) {
        // Set rotation to the matrix
        RotMatrix(&transforms->translation_rotation, &transforms->geometry_mtx);
        RotMatrix(&transforms->negative_translation_rotation, &transforms->frustum_mtx);
        // Divide out the fractions of camera coordinates and invert
        // the sign, so camera coordinates will line up to world
        // (or geometry) coordinates
        transforms->translation_position = vec3_i32(
            -camera->position.vx >> FIXED_POINT_SHIFT,
            -camera->position.vy >> FIXED_POINT_SHIFT,
            -camera->position.vz >> FIXED_POINT_SHIFT
        );
        transforms->negative_translation_position = vec3_i32(
            camera->position.vx >> FIXED_POINT_SHIFT,
            camera->position.vy >> FIXED_POINT_SHIFT,
            camera->position.vz >> FIXED_POINT_SHIFT
        );
        // Apply rotation of matrix to translation value to achieve a
        // first person perspective
        ApplyMatrixLV(
            &transforms->geometry_mtx,
            &transforms->translation_position,
            &transforms->translation_position
        );
        ApplyMatrixLV(
            &transforms->frustum_mtx,
            &transforms->negative_translation_position,
            &transforms->negative_translation_position
        );
        // Set translation matrix
        TransMatrix(&transforms->geometry_mtx,&transforms->translation_position);
        TransMatrix(&transforms->frustum_mtx,&transforms->negative_translation_position);
        // printf("[CAMERA] Geometry Matrix: \n" MAT_PATTERN, MAT_LAYOUT(transforms->geometry_mtx));
        // printf("[CAMERA] Frustum Matrix: \n" MAT_PATTERN, MAT_LAYOUT(transforms->frustum_mtx));
    }
}
