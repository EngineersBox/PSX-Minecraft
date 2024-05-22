#include "camera.h"

#include <psxapi.h>
#include <psxgpu.h>

#include "../blocks/block.h"
#include "../math/math_utils.h"

Camera cameraCreate(Transforms* transforms) {
    return (Camera) {
        .transforms = transforms,
        .frustum = frustumCreate(),
        .position = vec3_i32_all(0),
        .rotation = vec3_i32_all(0),
        .mode = 0
    };
}

void handleDigitalPadAndDualAnalogShock(Camera* camera, const Input* input, const Transforms* transforms) {
    // For digital input->pad, dual-analog and dual-shock
    if (input->pad->type != 0x4
        && input->pad->type != 0x5
        && input->pad->type != 0x7) {
        return;
    }
    // The button status bits are inverted,
    // so 0 means pressed in this case

    // // Look controls
    // if (isPressed(input->pad, binding_look_up)) {
    //     // Look up
    //     camera->rotation.vx = positiveModulo(camera->rotation.vx - (ONE * CAMERA_ROTATE_SPEED), ONE << FIXED_POINT_SHIFT);
    // } else if (isPressed(input->pad, binding_look_down)) {
    //     // Look down
    //     camera->rotation.vx = positiveModulo(camera->rotation.vx + (ONE * CAMERA_ROTATE_SPEED), ONE << FIXED_POINT_SHIFT);
    // }
    // if (isPressed(input->pad, binding_look_left)) {
    //     // Look left
    //     camera->rotation.vy = positiveModulo(camera->rotation.vy + (ONE * CAMERA_ROTATE_SPEED), ONE << FIXED_POINT_SHIFT);
    // } else if (isPressed(input->pad, binding_look_right)) {
    //     // Look right
    //     camera->rotation.vy = positiveModulo(camera->rotation.vy - (ONE * CAMERA_ROTATE_SPEED), ONE << FIXED_POINT_SHIFT);
    // }
    // // Movement controls
    // if (isPressed(input->pad, binding_move_forward)) {
    //     // Move forward
    //     camera->position.vx -= ((isin(transforms->translation_rotation.vy)
    //                              * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << CAMERA_MOVE_SPEED;
    //     camera->position.vz += ((icos(transforms->translation_rotation.vy)
    //                              * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << CAMERA_MOVE_SPEED;
    // } else if (isPressed(input->pad, binding_move_backward)) {
    //     // Move backward
    //     camera->position.vx += ((isin(transforms->translation_rotation.vy)
    //                              * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << CAMERA_MOVE_SPEED;
    //     camera->position.vz -= ((icos(transforms->translation_rotation.vy)
    //                              * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << CAMERA_MOVE_SPEED;
    // }
    // if (isPressed(input->pad, binding_move_left)) {
    //     // Slide left
    //     camera->position.vx -= icos(transforms->translation_rotation.vy) << CAMERA_MOVE_SPEED;
    //     camera->position.vz -= isin(transforms->translation_rotation.vy) << CAMERA_MOVE_SPEED;
    // } else if (isPressed(input->pad, binding_move_right)) {
    //     // Slide right
    //     camera->position.vx += icos(transforms->translation_rotation.vy) << CAMERA_MOVE_SPEED;
    //     camera->position.vz += isin(transforms->translation_rotation.vy) << CAMERA_MOVE_SPEED;
    // }
    // if (isPressed(input->pad, binding_jump)) {
    //     // Slide up
    //     camera->position.vy -= icos(transforms->translation_rotation.vx) << CAMERA_MOVE_SPEED;
    // } else if (isPressed(input->pad, binding_sneak)) {
    //     // Slide down
    //     camera->position.vy += icos(transforms->translation_rotation.vx) << CAMERA_MOVE_SPEED;
    // }
    if (isPressed(input->pad, PAD_SELECT)) {
        _boot();
    }
}

void handleDualAnalogShock(Camera* camera, const Input* input, const Transforms* transforms) {
    // For dual-analog and dual-shock (analog input)
    if ((input->pad->type != 0x5) && (input->pad->type != 0x7)) {
        return;
    }
    // // Moving forwards and backwards
    // if (((input->pad->ls_y - 128) < -16) || ((input->pad->ls_y - 128) > 16)) {
    //     camera->position.vx += (((isin(transforms->translation_rotation.vy)
    //                               * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT)
    //                             * (input->pad->ls_y - 128)) >> 5;
    //     camera->position.vy -= (isin(transforms->translation_rotation.vx) * (input->pad->ls_y - 128)) >> 5;
    //     camera->position.vz -= (((icos(transforms->translation_rotation.vy)
    //                               * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT)
    //                             * (input->pad->ls_y - 128)) >> 5;
    // }
    // // Strafing left and right
    // if (((input->pad->ls_x - 128) < -16) || ((input->pad->ls_x - 128) > 16)) {
    //     camera->position.vx += (icos(transforms->translation_rotation.vy) * (input->pad->ls_x - 128)) >> 5;
    //     camera->position.vz += (isin(transforms->translation_rotation.vy) * (input->pad->ls_x - 128)) >> 5;
    // }
    // Look up and down
    if (((input->pad->rs_y - 128) < -16) || ((input->pad->rs_y - 128) > 16)) {
        camera->rotation.vx += (input->pad->rs_y - 128) << 9;
    }
    // Look left and right
    if (((input->pad->rs_x - 128) < -16) || ((input->pad->rs_x - 128) > 16)) {
        camera->rotation.vy -= (input->pad->rs_x - 128) << 9;
    }
}

void cameraUpdate(Camera* camera, const Input* input, Transforms* transforms, const VECTOR* look_pos) {
    // Parse controller input
    camera->mode = 0;
    // Divide out fractions of camera rotation
    transforms->translation_rotation.vx = camera->rotation.vx >> FIXED_POINT_SHIFT;
    transforms->translation_rotation.vy = camera->rotation.vy >> FIXED_POINT_SHIFT;
    transforms->translation_rotation.vz = camera->rotation.vz >> FIXED_POINT_SHIFT;
    if (input->pad->stat == 0) {
        handleDigitalPadAndDualAnalogShock(camera, input, transforms);
        handleDualAnalogShock(camera, input, transforms);
    }
    // First-person camera mode
    if (camera->mode == 0) {
        // Set rotation to the matrix
        RotMatrix(&transforms->translation_rotation, &transforms->geometry_mtx);
        // Divide out the fractions of camera coordinates and invert
        // the sign, so camera coordinates will line up to world
        // (or geometry) coordinates
        transforms->translation_position.vx = -camera->position.vx >> FIXED_POINT_SHIFT;
        transforms->translation_position.vy = -camera->position.vy >> FIXED_POINT_SHIFT;
        transforms->translation_position.vz = -camera->position.vz >> FIXED_POINT_SHIFT;
        // Apply rotation of matrix to translation value to achieve a
        // first person perspective
        ApplyMatrixLV(
            &transforms->geometry_mtx,
            &transforms->translation_position,
            &transforms->translation_position
        );
        // Set translation matrix
        TransMatrix(
            &transforms->geometry_mtx,
            &transforms->translation_position
        );
    } else {
        // DEBUG: Tracking mode
        // Vector that defines the 'up' direction of the camera
        const SVECTOR up = {0, -ONE, 0};
        // Divide out fractions of camera coordinates
        transforms->translation_position.vx = camera->position.vx >> FIXED_POINT_SHIFT;
        transforms->translation_position.vy = camera->position.vy >> FIXED_POINT_SHIFT;
        transforms->translation_position.vz = camera->position.vz >> FIXED_POINT_SHIFT;
        // Look at the cube
        lookAt(
            &transforms->translation_position,
            look_pos,
            &up,
            &transforms->geometry_mtx
        );
    }
}

void lookAt(const VECTOR* eye, const VECTOR* at, const SVECTOR* up, MATRIX* mtx) {
    VECTOR taxis = vector_sub(*at, *eye);
    SVECTOR zaxis;
    SVECTOR xaxis;
    SVECTOR yaxis;
    VectorNormalS(&taxis, &zaxis);
    _crossProduct(&zaxis, up, &taxis);
    VectorNormalS(&taxis, &xaxis);
    _crossProduct(&zaxis, &xaxis, &taxis);
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

bool cameraInputHandler(const Input* input, void* ctx) {
    Camera* camera = (Camera*) ctx;
    Transforms* transforms = camera->transforms;
    // Divide out fractions of camera rotation
    transforms->translation_rotation.vx = camera->rotation.vx >> FIXED_POINT_SHIFT;
    transforms->translation_rotation.vy = camera->rotation.vy >> FIXED_POINT_SHIFT;
    transforms->translation_rotation.vz = camera->rotation.vz >> FIXED_POINT_SHIFT;
    if (input->pad->stat == 0) {
        handleDigitalPadAndDualAnalogShock(camera, input, transforms);
        handleDualAnalogShock(camera, input, transforms);
    }
    // First-person camera mode
    if (camera->mode == 0) {
        // Set rotation to the matrix
        RotMatrix(&transforms->translation_rotation, &transforms->geometry_mtx);
        // Divide out the fractions of camera coordinates and invert
        // the sign, so camera coordinates will line up to world
        // (or geometry) coordinates
        transforms->translation_position.vx = -camera->position.vx >> FIXED_POINT_SHIFT;
        transforms->translation_position.vy = -camera->position.vy >> FIXED_POINT_SHIFT;
        transforms->translation_position.vz = -camera->position.vz >> FIXED_POINT_SHIFT;
        // Apply rotation of matrix to translation value to achieve a
        // first person perspective
        ApplyMatrixLV(
            &transforms->geometry_mtx,
            &transforms->translation_position,
            &transforms->translation_position
        );
        // Set translation matrix
        TransMatrix(
            &transforms->geometry_mtx,
            &transforms->translation_position
        );
    }
    // Camera is the base level input handler, we should always give up
    // control to a layer added on top of base movement
    return false;
}

void cameraRegisterInputHandler(VSelf, Input* input) ALIAS("Camera_registerInputHandler");
void Camera_registerInputHandler(VSelf, Input* input) {
    VSELF(Camera);
    const ContextualInputHandler handler = (ContextualInputHandler) {
        .ctx = self,
        .input_handler = cameraInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}
