#include "camera.h"

#include <psxapi.h>
#include <psxgpu.h>

#include "../util/math_utils.h"

#define isPressed(pad_button) (!(input->pad->btn & (pad_button)))

void printDebugCamera(const Camera *camera, const Input *input) {
    // Print out some info
    FntPrint(-1, "BUTTONS=%04x\n", input->pad->btn);
    FntPrint(
        -1,
        "X=%d Y=%d Z=%d\n",
        camera->position.vx >> FIXED_POINT_SHIFT,
        camera->position.vy >> FIXED_POINT_SHIFT,
        camera->position.vz >> FIXED_POINT_SHIFT
    );
    FntPrint(
        -1,
        "RX=%d RY=%d\n",
        camera->rotation.vx >> FIXED_POINT_SHIFT,
        camera->rotation.vy >> FIXED_POINT_SHIFT
    );
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

    // Look controls
    if (isPressed(PAD_UP)) {
        // Look up
        camera->rotation.vx -= ONE * 8;
    } else if (isPressed(PAD_DOWN)) {
        // Look down
        camera->rotation.vx += ONE * 8;
    }
    if (isPressed(PAD_LEFT)) {
        // Look left
        camera->rotation.vy += ONE * 8;
    } else if (isPressed(PAD_RIGHT)) {
        // Look right
        camera->rotation.vy -= ONE * 8;
    }
    // Movement controls
    if (isPressed(PAD_TRIANGLE)) {
        // Move forward
        camera->position.vx -= ((isin(transforms->translation_rotation.vy)
                                 * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
        camera->position.vy += isin(transforms->translation_rotation.vx) << 2;
        camera->position.vz += ((icos(transforms->translation_rotation.vy)
                                 * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
    } else if (isPressed(PAD_CROSS)) {
        // Move backward
        camera->position.vx += ((isin(transforms->translation_rotation.vy)
                                 * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
        camera->position.vy -= isin(transforms->translation_rotation.vx) << 2;
        camera->position.vz -= ((icos(transforms->translation_rotation.vy)
                                 * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
    }
    if (isPressed(PAD_SQUARE)) {
        // Slide left
        camera->position.vx -= icos(transforms->translation_rotation.vy) << 2;
        camera->position.vz -= isin(transforms->translation_rotation.vy) << 2;
    } else if (isPressed(PAD_CIRCLE)) {
        // Slide right
        camera->position.vx += icos(transforms->translation_rotation.vy) << 2;
        camera->position.vz += isin(transforms->translation_rotation.vy) << 2;
    }
    if (isPressed(PAD_R1)) {
        // Slide up
        camera->position.vx -= ((isin(transforms->translation_rotation.vy)
                                 * isin(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
        camera->position.vy -= icos(transforms->translation_rotation.vx) << 2;
        camera->position.vz += ((icos(transforms->translation_rotation.vy)
                                 * isin(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
    } else if (isPressed(PAD_R2)) {
        // Slide down
        camera->position.vx += ((isin(transforms->translation_rotation.vy)
                                 * isin(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
        camera->position.vy += icos(transforms->translation_rotation.vx) << 2;
        camera->position.vz -= ((icos(transforms->translation_rotation.vy)
                                 * isin(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT) << 2;
    }
    // DEBUG: Look at cube
    if (isPressed(PAD_L1)) {
        camera->mode = 1;
    }
    if (isPressed(PAD_START)) {
        // BUG: This causes an crash with an invalid address for some reaosn
        camera->reset_handler(camera);
    }
    if (isPressed(PAD_SELECT)) {
        _boot();
    }
}

void handleDualAnalogShock(Camera *camera, const Input *input, const Transforms *transforms) {
    // For dual-analog and dual-shock (analog input)
    if ((input->pad->type != 0x5) && (input->pad->type != 0x7)) {
        return;
    }
    // Moving forwards and backwards
    if (((input->pad->ls_y - 128) < -16) || ((input->pad->ls_y - 128) > 16)) {
        camera->position.vx += (((isin(transforms->translation_rotation.vy)
                                  * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT)
                                * (input->pad->ls_y - 128)) >> 5;
        camera->position.vy -= (isin(transforms->translation_rotation.vx) * (input->pad->ls_y - 128)) >> 5;
        camera->position.vz -= (((icos(transforms->translation_rotation.vy)
                                  * icos(transforms->translation_rotation.vx)) >> FIXED_POINT_SHIFT)
                                * (input->pad->ls_y - 128)) >> 5;
    }
    // Strafing left and right
    if (((input->pad->ls_x - 128) < -16) || ((input->pad->ls_x - 128) > 16)) {
        camera->position.vx += (icos(transforms->translation_rotation.vy) * (input->pad->ls_x - 128)) >> 5;
        camera->position.vz += (isin(transforms->translation_rotation.vy) * (input->pad->ls_x - 128)) >> 5;
    }
    // Look up and down
    if (((input->pad->rs_y - 128) < -16) || ((input->pad->rs_y - 128) > 16)) {
        camera->rotation.vx += (input->pad->rs_y - 128) << 9;
    }
    // Look left and right
    if (((input->pad->rs_x - 128) < -16) || ((input->pad->rs_x - 128) > 16)) {
        camera->rotation.vy -= (input->pad->rs_x - 128) << 9;
    }
}

void cameraUpdate(Camera *camera, const Input *input, Transforms *transforms, const VECTOR *look_pos) {
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
    printDebugCamera(camera, input);
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

void lookAt(const VECTOR *eye, const VECTOR *at, const SVECTOR *up, MATRIX *mtx) {
    VECTOR taxis;
    SVECTOR zaxis;
    SVECTOR xaxis;
    SVECTOR yaxis;
    setVector(&taxis, at->vx-eye->vx, at->vy-eye->vy, at->vz-eye->vz);
    VectorNormalS(&taxis, &zaxis);
    crossProduct(&zaxis, up, &taxis);
    VectorNormalS(&taxis, &xaxis);
    crossProduct(&zaxis, &xaxis, &taxis);
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
