#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <psxgte.h>
#include <psxpad.h>

typedef struct {
    VECTOR position;
    VECTOR rotation;
    int mode;
} Camera;

void updateCamera(PADTYPE* pad);

#endif //CAMERA_H
