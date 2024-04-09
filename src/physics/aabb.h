#pragma once

#ifndef PSX_MINECRAFT_AABB_H
#define PSX_MINECRAFT_AABB_H

#include <stdbool.h>
#include <stdint.h>

#include "../util/inttypes.h"

typedef struct {
    i32 min_x;
    i32 max_x;
    i32 min_y;
    i32 max_y;
    i32 min_z;
    i32 max_z;
} AABB;

bool aabbIntersect(const AABB* a, const AABB* b);
void aabbOffset(AABB* aabb, i32 x, i32 y, i32 z);

#endif // PSX_MINECRAFT_AABB_H
