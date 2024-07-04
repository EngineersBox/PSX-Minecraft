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

/**
 * @brief Computes the direction opposing the given one. I.e returns
 *        @code FACE_DIR_LEFT@endcode if given @code FACE_DIR_RIGHT@endcode.
 *
 * @note Using bitwise XOR here, allows us to flip the least significant
 *       bit in the number. If we think of the face directions as opposing
 *       sequential pairs then apply XOR on the bitwise representations,
 *       we see the desired outcome:
 *       @code
 *       Name  | Index | XOR'd Index
 *       ------|-------|----------------------
 *       DOWN  | 0b000 | 0b000 ^ 0b001 = 0b001
 *       UP    | 0b001 | 0b001 ^ 0b001 = 0b000
 *       LEFT  | 0b010 | 0b010 ^ 0b001 = 0b011
 *       RIGHT | 0b011 | 0b011 ^ 0b001 = 0b010
 *       BACK  | 0b100 | 0b100 ^ 0b001 = 0b101
 *       FRONT | 0b101 | 0b101 ^ 0b001 = 0b100
 *       @endcode
 *
 * @param face_dir Current @code FaceDirection@endcode
 * @return Opposing @code FaceDirection@endcode
 */
#define faceDirectionOpposing(face_dir) ((FaceDirection) (((u8) (face_dir)) ^ 1))

// [-1,1] => [0,1]
#define convertAxis(v,a) (((v).a + 1) >> 1)
/**
* @brief Converts a normal vector into its equivalent @code FaceDirection@endcode
* @param normal Normal vector instance of @code VECTOR@endcode,
*               @code SVECTOR@endcode or @code CVECTOR@endcode
* @return @code FaceDirection@endcode corresponding to normal
*/
#define faceDirectionFromNormal(normal) \
      ((convertAxis(normal, vy) | 0b000) * ((normal).vy & 0b1)) \
    | ((convertAxis(normal, vx) | 0b010) * ((normal).vx & 0b1)) \
    | ((convertAxis(normal, vz) | 0b100) * ((normal).vz & 0b1))

/**
* @brief For a given direction vector, finds the closest @code FaceDirection@endcode
* @param vec Normalised direction @code VECTOR@endcode instance
* @return @code FaceDirection@endcode corresponding to closes direction
*/
FaceDirection faceDirectionClosestNormal(const VECTOR vec);

// Given target_direction, a direction relative to FACE_DIR_FRONT,
// this computes the equivalent direction of target_direction
// relevant to front_direction, right_direction, up_direction.
FaceDirection faceDirectionRelative(FaceDirection target_direction,
                                    FaceDirection front_direction,
                                    FaceDirection right_direction,
                                    FaceDirection up_direction);

#endif // _PSX_MINECRAFT__STRUCTURE_PRIMITIVE__DIRECTION_H_
