#include "plane.h"

#include "../math/math_utils.h"

Plane planeCreate(VECTOR p1, VECTOR normal) {
    return (Plane) {
        .normal = normal,
        .distance = dot(&normal, &p1)
    };
}
