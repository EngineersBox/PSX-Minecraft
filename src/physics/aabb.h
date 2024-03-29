#pragma once

#ifndef PSX_MINECRAFT_AABB_H
#define PSX_MINECRAFT_AABB_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t min_x;
    int16_t max_x;
    int16_t min_y;
    int16_t max_y;
    int16_t min_z;
    int16_t max_z;
} AABB;

bool aabbIntersect(const AABB* a, const AABB* b);

#endif // PSX_MINECRAFT_AABB_H
