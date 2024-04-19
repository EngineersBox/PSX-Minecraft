#pragma once

#ifndef PSX_MINECRAFT_AABB_H
#define PSX_MINECRAFT_AABB_H

#include <stdbool.h>
#include <psxgte.h>

#include "../util/inttypes.h"

typedef struct {
    VECTOR min;
    VECTOR max;
} AABB;

void aabbAddCoord(AABB* source, AABB* target, i32 x, i32 y, i32 z);
void aabbOffset(AABB* aabb, i32 x, i32 y, i32 z);

i32 aabbYOffset(AABB* source, AABB* target, i32 y);
i32 aabbXOffset(AABB* source, AABB* target, i32 x);
i32 aabbZOffset(AABB* source, AABB* target, i32 z);

bool aabbIntersects(const AABB* source, const AABB* target);

#endif // PSX_MINECRAFT_AABB_H
