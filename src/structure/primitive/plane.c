#include "plane.h"

#include "../math/math_utils.h"

bool planePointInFront(const Plane* plane, const VECTOR point) {
    // Dot between plane normal and point compute the distance between
    // the two, relative to the origin. We then compare this to the offset
    // distance that the plane is at from the origin, which tells us
    // whether the distance of the point from the origin is more than
    // the distance that the plane is away from the origin determining
    // whether the point is in front of the plane in the normal direction
    return dot_i64(plane->normal, point) >= plane->distance;
}