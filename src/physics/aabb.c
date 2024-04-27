#include "aabb.h"

#include "../math/math_utils.h"

void aabbAddCoord(const AABB* source, AABB* target, const i32 x, const i32 y, const i32 z) {
    target->min = (VECTOR) {
        .vx = source->min.vx + max(x, 0),
        .vy = source->min.vy + max(y, 0),
        .vz = source->min.vz + max(z, 0)
    };
    target->max = (VECTOR) {
        .vx = source->max.vx + min(x, 0),
        .vy = source->max.vy + min(y, 0),
        .vz = source->max.vz + min(z, 0)
    };
}

void aabbOffset(AABB* aabb, const i32 x, const i32 y, const i32 z) {
    aabb->min.vx += x;
    aabb->min.vy += y;
    aabb->min.vz += z;
    aabb->max.vx += x;
    aabb->max.vy += y;
    aabb->max.vz += z;
}

i32 aabbYOffset(const AABB* source, const AABB* target, const i32 y) {
    if (target->max.vx <= source->min.vx || target->min.vx >= source->max.vx) {
        return y;
    } else if (target->max.vz <= source->min.vz || target->min.vz >= source->max.vz) {
        return y;
    }
    if (y > 0 && target->max.vy <= source->min.vy) {
        const i32 diff_y = source->min.vy - target->max.vy;
        if (diff_y < y) {
            return diff_y;
        }
    }
    if (y < 0 && target->min.vy >= source->max.vy) {
        const i32 diff_y = source->max.vy - target->min.vy;
        if (diff_y > y) {
            return diff_y;
        }
    }
    return y;
}

i32 aabbXOffset(const AABB* source, const AABB* target, const i32 x) {
    if (target->max.vy <= source->min.vy || target->min.vy >= source->max.vy) {
        return x;
    } else if (target->max.vz <= source->min.vz || target->min.vz >= source->max.vz) {
        return x;
    }
    if (x > 0.0 && target->max.vx <= source->min.vx) {
        const i32 diff_x = source->min.vx - target->max.vx;
        if (diff_x < x) {
            return  diff_x;
        }
    }
    if (x < 0.0 && target->min.vx >= source->max.vx) {
        const i32 diff_x = source->max.vx - target->min.vx;
        if (diff_x > x) {
            return diff_x;
        }
    }
    return x;
}

i32 aabbZOffset(const AABB* source, const AABB* target, i32 z) {
    if (target->max.vx <= source->min.vx || target->min.vx >= source->max.vx) {
        return z;
    } else if (target->max.vy <= source->min.vy || target->min.vy >= source->max.vy) {
        return z;
    }
    if (z > 0 && target->max.vz <= source->min.vz) {
        const i32 diff_z = source->min.vz - target->max.vz;
        if (diff_z < z) {
            return diff_z;
        }
    }
    if (z < 0 && target->min.vz >= source->max.vz) {
        const i32 diff_z = source->max.vz - target->min.vz;
        if (diff_z > z) {
            return diff_z;
        }
    }
    return z;
}

bool aabbIntersects(const AABB* a, const AABB* b) {
    return a->min.vx < b->max.vx
        && a->max.vx > b->min.vx
        && a->min.vy < b->max.vy
        && a->max.vy > b->min.vy
        && a->min.vz < b->max.vz
        && a->max.vz > b->min.vz;
}