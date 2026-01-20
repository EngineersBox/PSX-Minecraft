#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <psxgte.h>

#include "input/input.h"
#include "input/bindings.h"
#include "../render/transforms.h"

#define CAMERA_ROTATE_SPEED 15
#define CAMERA_MOVE_SPEED 3

typedef enum CameraMode {
    CAMERA_MODE_FIRST_PERSON = 0,
    CAMERA_MODE_THIRD_PERSON = 1
} CameraMode;

typedef struct {
    Transforms* transforms;
    VECTOR position;
    // Facing direction
    VECTOR rotation;
    // READ ONLY
    // Equivalent vector direction
    // for rotation
    VECTOR direction;
    CameraMode mode;
} Camera;

Camera cameraCreate(Transforms* transforms);

void lookAt(const VECTOR* eye, const VECTOR* at, const SVECTOR* up, MATRIX* mtx);
void cameraUpdate(Camera* camera);

#endif //CAMERA_H
