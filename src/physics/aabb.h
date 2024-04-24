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

void aabbAddCoord(const AABB* source, AABB* target, const i32 x, const i32 y, const i32 z);
void aabbOffset(AABB* aabb, const i32 x, const i32 y, const i32 z);

i32 aabbYOffset(const AABB* source, const AABB* target, i32 y);
i32 aabbXOffset(const AABB* source, const AABB* target, i32 x);
i32 aabbZOffset(const AABB* source, const AABB* target, i32 z);

bool aabbIntersects(const AABB* a, const AABB* b);

#endif // PSX_MINECRAFT_AABB_H
