#include "camera.h"

#include <psxapi.h>
#include <psxgte.h>

#include "../logging/logging.h"
#include "../game/blocks/block.h"
#include "../math/vector.h"

Camera cameraCreate(Transforms* transforms) {
    return (Camera) {
        .transforms = transforms,
        .position = vec3_i32(0),
        .rotation = vec3_i32(0),
        .direction = vec3_i32(0),
        .mode = CAMERA_MODE_FIRST_PERSON
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
    // First-person camera mode
    if (camera->mode == CAMERA_MODE_FIRST_PERSON) {
        // Set rotation to the matrix
        RotMatrix(&transforms->translation_rotation, &transforms->geometry_mtx);
        RotMatrix(&transforms->translation_rotation, &transforms->skybox_mtx);
        // InvRotMatrix(&transforms->negative_translation_rotation, &transforms->frustum_mtx);
        // Divide out the fractions of camera coordinates and invert
        // the sign, so camera coordinates will line up to world
        // (or geometry) coordinates
        transforms->translation_position = vec3_i32(
            -camera->position.vx >> FIXED_POINT_SHIFT,
            -camera->position.vy >> FIXED_POINT_SHIFT,
            -camera->position.vz >> FIXED_POINT_SHIFT
        );
        VECTOR zero_pos = VEC3_I32_ZERO;
        // Apply rotation of matrix to translation value to achieve a
        // first person perspective
        ApplyMatrixLV(
            &transforms->geometry_mtx,
            &transforms->translation_position,
            &transforms->translation_position
        );
        ApplyMatrixLV(&transforms->skybox_mtx, &zero_pos, &zero_pos);
        // Set translation matrix
        TransMatrix(&transforms->geometry_mtx,&transforms->translation_position);
        TransMatrix(&transforms->skybox_mtx, &zero_pos);
    }
}
