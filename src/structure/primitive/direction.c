#include "direction.h"

#include <preprocessor.h>

#include "../math/vector.h"

const SVECTOR FACE_DIRECTION_NORMALS[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0, 1, 0),
    [FACE_DIR_UP] = vec3_i16(0, -1, 0),
    [FACE_DIR_LEFT] = vec3_i16(-1, 0, 0),
    [FACE_DIR_RIGHT] = vec3_i16(1, 0, 0),
    [FACE_DIR_BACK] = vec3_i16(0, 0, 1),
    [FACE_DIR_FRONT] = vec3_i16(0, 0, -1)
};

FaceDirection faceDirectionRelative(FaceDirection target_direction,
                                    FaceDirection front_direction,
                                    FaceDirection right_direction,
                                    FaceDirection up_direction) {
    UNIMPLEMENTED();
}