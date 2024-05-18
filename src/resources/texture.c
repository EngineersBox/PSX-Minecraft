#include "texture.h"

#include "../math/math_utils.h"

const SVECTOR FACE_DIRECTION_NORMALS[6] = {
    vec3_i16(0, 1, 0),
    vec3_i16(0, -1, 0),
    vec3_i16(-1, 0, 0),
    vec3_i16(1, 0, 0),
    vec3_i16(0, 0, 1),
    vec3_i16(0, 0, -1)
};