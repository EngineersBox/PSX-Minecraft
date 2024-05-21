#include "plane.h"

#include <logging.h>

#include "../math/math_utils.h"

Plane planeCreate(const VECTOR point, const VECTOR normal) {
    DEBUG_LOG(
        "[PLANE] Point: (%d,%d,%d) Normal: (%d,%d,%d) Distance: %d\n",
        inlineVec(point),
        inlineVec(normal),
        dot(&normal, &point)
    );
    return (Plane) {
        .normal = normal,
        .distance = _dot(normal, point)
    };
}