#pragma once

#ifndef _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__DIRECTION_H_
#define _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__DIRECTION_H_

#include <psxgte.h>

#include "../util/inttypes.h"

typedef enum {
    FACE_DIR_DOWN = 0,
    FACE_DIR_UP,
    FACE_DIR_LEFT,
    FACE_DIR_RIGHT,
    FACE_DIR_BACK,
    FACE_DIR_FRONT
} FaceDirection;

#define FACE_DIRECTION_COUNT 6

extern const SVECTOR FACE_DIRECTION_NORMALS[FACE_DIRECTION_COUNT];

// Given target_direction, a direction relative to FACE_DIR_FRONT,
// this computes the equivalent direction of target_direction
// relevant to front_direction, right_direction, up_direction.
FaceDirection faceDirectionRelative(FaceDirection target_direction,
                                    FaceDirection front_direction,
                                    FaceDirection right_direction,
                                    FaceDirection up_direction);

#endif // _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__DIRECTION_H_