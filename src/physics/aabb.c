#include "aabb.h"

void aabbAddCoord(AABB* source, AABB* target, i32 x, i32 y, i32 z) {
    target->min = (VECTOR) {
        .vx = source->min.vx + (x < 0 ? x : 0),
        .vy = source->min.vy + (y < 0 ? y : 0),
        .vz = source->min.vz + (z < 0 ? z : 0)
    };
    target->max = (VECTOR) {
        .vx = source->max.vx + (x > 0 ? x : 0),
        .vy = source->max.vy + (y > 0 ? y : 0),
        .vz = source->max.vz + (z > 0 ? z : 0)
    };
}

void aabbOffset(AABB* aabb, i32 x, i32 y, i32 z) {
    aabb->min.vx += x;
    aabb->min.vy += y;
    aabb->min.vz += z;
    aabb->max.vx += x;
    aabb->max.vy += y;
    aabb->max.vz += z;
}

i32 aabbYOffset(AABB* source, AABB* target, i32 y) {
    // TODO: Simplify this nesting
    if (target->max.vx > source->min.vx && target->min.vx < source->max.vx) {
        if (target->max.vz > source->min.vz && target->min.vz < source->max.vz) {
            i32 diff_y;
            if(y > 0 && target->max.vy <= source->min.vy) {
                diff_y = source->min.vy - target->max.vy;
                if(diff_y < y) {
                    y = diff_y;
                }
            }
            if(y < 0 && target->min.vy >= source->max.vy) {
                diff_y = source->max.vy - target->min.vy;
                if(diff_y > y) {
                    y = diff_y;
                }
            }
        }
    }
    return y;
}

i32 aabbXOffset(AABB* source, AABB* target, i32 x) {
    if(target->max.vy > source->min.vy && target->min.vy < source->max.vy) {
        if(target->max.vz > source->min.vz && target->min.vz < source->max.vz) {
            double diff_x;
            if(x > 0 && target->max.vx <= source->min.vx) {
                diff_x = source->min.vx - target->max.vx;
                if(diff_x < x) {
                    x = diff_x;
                }
            }

            if(x < 0 && target->min.vx >= source->max.vx) {
                diff_x = source->max.vx - target->min.vx;
                if(diff_x > x) {
                    x = diff_x;
                }
            }

        }
    }
    return x;
}

i32 aabbZOffset(AABB* source, AABB* target, i32 z) {
    if(target->max.vx > source->min.vx && target->min.vx < source->max.vx) {
        if(target->max.vy > source->min.vy && target->min.vy < source->max.vy) {
            double diff_z;
            if(z > 0 && target->max.vz <= source->min.vz) {
                diff_z = source->min.vz - target->max.vz;
                if(diff_z < z) {
                    z = diff_z;
                }
            }

            if(z < 0 && target->min.vz >= source->max.vz) {
                diff_z = source->max.vz - target->min.vz;
                if(diff_z > z) {
                    z = diff_z;
                }
            }
        }
    }
    return z;
}

bool aabbIntersects(const AABB* source, const AABB* target) {
    return target->max.vx > source->min.vx
        && target->min.vx < source->max.vx
        && target->max.vy > source->min.vy
        && target->min.vy < source->max.vy
        && target->max.vz > source->min.vz
        && target->min.vz < source->max.vz;
}