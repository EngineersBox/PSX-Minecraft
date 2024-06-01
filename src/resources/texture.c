#include "texture.h"

#include "../math/vector.h"

const SVECTOR FACE_DIRECTION_NORMALS[6] = {
    [FACE_DIR_DOWN] = vec3_i16(0, 1, 0),
    [FACE_DIR_UP] = vec3_i16(0, -1, 0),
    [FACE_DIR_LEFT] = vec3_i16(-1, 0, 0),
    [FACE_DIR_RIGHT] = vec3_i16(1, 0, 0),
    [FACE_DIR_BACK] = vec3_i16(0, 0, 1),
    [FACE_DIR_FRONT] = vec3_i16(0, 0, -1)
};