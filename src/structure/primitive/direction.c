#include "direction.h"

#include "../../logging/logging.h"
#include "../../math/math_utils.h"
#include "../../math/vector.h"

const SVECTOR FACE_DIRECTION_NORMALS[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0, 1, 0),
    [FACE_DIR_UP] = vec3_i16(0, -1, 0),
    [FACE_DIR_LEFT] = vec3_i16(-1, 0, 0),
    [FACE_DIR_RIGHT] = vec3_i16(1, 0, 0),
    [FACE_DIR_BACK] = vec3_i16(0, 0, -1),
    [FACE_DIR_FRONT] = vec3_i16(0, 0, 1)
};
const SVECTOR FACE_DIRECTION_NORMALS_UNIT[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0, ONE, 0),
    [FACE_DIR_UP] = vec3_i16(0, -ONE, 0),
    [FACE_DIR_LEFT] = vec3_i16(-ONE, 0, 0),
    [FACE_DIR_RIGHT] = vec3_i16(ONE, 0, 0),
    [FACE_DIR_BACK] = vec3_i16(0, 0, -ONE),
    [FACE_DIR_FRONT] = vec3_i16(0, 0, ONE)
};
const SVECTOR FACE_DIRECTION_NORMALS_WORLD[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0, -1, 0),
    [FACE_DIR_UP] = vec3_i16(0, 1, 0),
    [FACE_DIR_LEFT] = vec3_i16(-1, 0, 0),
    [FACE_DIR_RIGHT] = vec3_i16(1, 0, 0),
    [FACE_DIR_BACK] = vec3_i16(0, 0, -1),
    [FACE_DIR_FRONT] = vec3_i16(0, 0, 1)
};
const SVECTOR FACE_DIRECTION_NORMALS_WORLD_UNIT[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0, -ONE, 0),
    [FACE_DIR_UP] = vec3_i16(0, ONE, 0),
    [FACE_DIR_LEFT] = vec3_i16(-ONE, 0, 0),
    [FACE_DIR_RIGHT] = vec3_i16(ONE, 0, 0),
    [FACE_DIR_BACK] = vec3_i16(0, 0, -ONE),
    [FACE_DIR_FRONT] = vec3_i16(0, 0, ONE)
};
const SVECTOR FACE_DIRECTION_RELATIVE_LEFT[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0),
    [FACE_DIR_UP] = vec3_i16(0),
    [FACE_DIR_LEFT] = vec3_i16(0, 0, -1),
    [FACE_DIR_RIGHT] = vec3_i16(0, 0, 1),
    [FACE_DIR_BACK] = vec3_i16(1, 0, 0),
    [FACE_DIR_FRONT] = vec3_i16(-1, 0, 0)
};
const SVECTOR FACE_DIRECTION_RELATIVE_RIGHT[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = vec3_i16(0),
    [FACE_DIR_UP] = vec3_i16(0),
    [FACE_DIR_LEFT] = vec3_i16(0, 0, 1),
    [FACE_DIR_RIGHT] = vec3_i16(0, 0, -1),
    [FACE_DIR_BACK] = vec3_i16(-1, 0, 0),
    [FACE_DIR_FRONT] = vec3_i16(1, 0, 0)
};

const INDEX FACE_DIRECTION_INDICES[FACE_DIRECTION_COUNT] = {
    [FACE_DIR_DOWN] = {2, 3, 6, 7},
    [FACE_DIR_UP] = {4, 5, 0, 1},
    [FACE_DIR_LEFT] = {4, 0, 6, 2},
    [FACE_DIR_RIGHT] = {1, 5, 3, 7},
    [FACE_DIR_BACK] = {5, 4, 7, 6},
    [FACE_DIR_FRONT] = {0, 1, 2, 3}
};

FaceDirection faceDirectionClosestNormal(const VECTOR vec) {
    // const VECTOR _vec = vec3_i32_normalize(vec);
    const i32 xn = absv(vec.vx);
    const i32 yn = absv(vec.vy);
    const i32 zn = absv(vec.vz);
    if (xn >= yn && xn >= zn) {
        return vec.vx > 0 ? FACE_DIR_RIGHT : FACE_DIR_LEFT;
    } else if (yn > xn && yn >= zn) {
        return vec.vy > 0 ? FACE_DIR_UP : FACE_DIR_DOWN;
    } else if (zn > xn && zn > yn) {
        return vec.vz > 0 ? FACE_DIR_FRONT : FACE_DIR_BACK;
    }
    return FACE_DIR_RIGHT;
}

FaceDirection faceDirectionRelative(UNUSED FaceDirection target_direction,
                                    UNUSED FaceDirection front_direction,
                                    UNUSED FaceDirection right_direction,
                                    UNUSED FaceDirection up_direction) {
    UNIMPLEMENTED();
    return 0;
}
