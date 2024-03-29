#include "aabb.h"

bool aabbIntersect(const AABB* a, const AABB* b) {
    return a->min_x <= b->max_x
        && a->max_x >= b->min_x
        && a->min_y <= b->max_y
        && a->max_y >= b->min_y
        && a->min_z <= b->max_z
        && a->max_z >= b->min_z;
}
