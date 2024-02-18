#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <psxgte.h>

#include "input.h"
#include "../render/transforms.h"

#define CAMERA_ROTATE_SPEED 10
#define CAMERA_MOVE_SPEED 2

struct Camera;

typedef void(*StartHandler)(struct Camera*);

typedef struct Camera {
    VECTOR position;
    VECTOR rotation;
    int mode;
    StartHandler start_handler;
} Camera;

void cameraUpdate(Camera* camera, const Input* input, Transforms* transforms, const VECTOR* look_pos);
void lookAt(const VECTOR* eye, const VECTOR* at, const SVECTOR* up, MATRIX* mtx);

#endif //CAMERA_H
