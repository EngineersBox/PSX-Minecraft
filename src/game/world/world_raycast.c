#include "world_raycast.h"

#include "../blocks/blocks.h"
#include "../../math/vector.h"
#include "../../util/interface99_extensions.h"

// Forward declaration
IBlock* worldGetBlock(const World* world, const VECTOR* position);

INLINE double signd(const double v) {
    return v < 0.0 ? -1.0 : 1.0;
}

INLINE double absd(const double v) {
    return v < 0 ? -v : v;
}

INLINE double signumd(const double x) {
    return x > 0.0 ? 1.0 : x < 0.0 ? -1.0 : 0.0;
}

INLINE double dmod(const double x, const double y) {
    return x - (long long)(x/y) * y;
}

INLINE double modd(const double value, const double modulus) {
    return dmod(dmod(value, modulus + modulus), modulus);
}

double floord(const double v) {
    const long long n = (long long) v;
    const double d = (double) n;
    if (d == v || v >= 0)
        return d;
    return d - 1;
}

double roundd(const double v) {
    const double v_floor = floord(v);
    const double capped = v - v_floor;
    if (v - capped <= 0.5) {
        return v_floor;
    }
    return v_floor + 1.0;
}

double ceild(const double v) {
    const long long n = (long long) v;
    const double d = (double) n;
    if (d == v || v >= 0)
        return d + 1;
    return d;
}

double intboundd(const double s, const double ds) {
    if (ds < 0 && roundd(s) == s) {
        return 0;
    }
    const double dividend = ds > 0
        ? ceild(s) - s
        : s - floord(s);
    return dividend / absd(ds);
}

// It works. Do I like it? No. Do I care? Also no.
RayCastResult worldRayCastIntersection(const World* world,
                                       const Camera* camera,
                                       const i32 radius) {
    const double origin[3] = {
        ((double) camera->position.vx) / ((double) ONE_BLOCK),
        ((double) -camera->position.vy) / ((double) ONE_BLOCK),
        ((double) camera->position.vz) / ((double) ONE_BLOCK)
    };
    const VECTOR _step = rotationToDirection5o(&camera->rotation);
    const double direction[3] = {
        ((double) _step.vx) / ((double) ONE),
        ((double) _step.vy) / ((double) ONE),
        ((double) _step.vz) / ((double) ONE)
    };
    // Cube containing origin point
    double x = floord(origin[0]);
    double y = floord(origin[1]);
    double z = floord(origin[2]);
    // Break out direction vector
    const double dx = direction[0];
    const double dy = direction[1];
    const double dz = direction[2];
    if (dx == 0 && dy == 0 && dz == 0) {
        return (RayCastResult) {
            .pos = vec3_i32_all(0),
            .block = NULL,
            .face = vec3_i32_all(0)
        };
    }
    // Direction to increment x,y,z when stepping
    const double stepX = signumd(dx);
    const double stepY = signumd(dy);
    const double stepZ = signumd(dz);
    // See description above. The initial values depend on the fractional
    // part of the origin.
    double tMaxX = intboundd(origin[0], dx);
    double tMaxY = intboundd(origin[1], dy);
    double tMaxZ = intboundd(origin[2], dz);
    // The change in t when taking a step (always positive).
    const double tDeltaX = stepX/dx;
    const double tDeltaY = stepY/dy;
    const double tDeltaZ = stepZ/dz;
    // Buffer for reporting faces to the callback.
    VECTOR face = vec3_i32_all(0);
    const double rad = ((double) radius * (double) radius) / (dx * dx + dy * dy + dz * dz);
    while (true) {
        // Check what the current block position is, if its non-empty
        // then return the result
        const VECTOR position = vec3_i32(
            (i32) x,
            (i32) y,
            (i32) z
        );
        IBlock* iblock = worldGetBlock(world, &position);
        if (iblock == NULL) {
            printf("[WORLD] Raycast enountered null block at " VEC_PATTERN "\n", VEC_LAYOUT(position));
            abort();
            return (RayCastResult) {};
        }
        const Block* block = VCAST_PTR(Block*, iblock);
        // TODO: Handle sub-block intersection test for blocks like doors and piston heads
        if (blockGetType(block->id) != BLOCKTYPE_EMPTY) {
            return (RayCastResult) {
                .pos = vec3_i32(position.vx, position.vy, position.vz),
                .block = iblock,
                .face = vec3_i32(face.vx * ONE, face.vy * ONE, face.vz * ONE)
            };
        }
        // tMaxX stores the t-value at which we cross a cube boundary along the
        // X axis, and similarly for Y and Z. Therefore, choosing the least tMax
        // chooses the closest cube boundary. Only the first case of the four
        // has been commented in detail.
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                if ((tMaxX * tMaxX) > rad) break;
                // Update which cube we are now in.
                x += stepX;
                // Adjust tMaxX to the next X-oriented boundary crossing.
                tMaxX += tDeltaX;
                // Record the normal vector of the cube face we entered.
                face.vx = -stepX;
                face.vy = 0;
                face.vz = 0;
            } else {
                if ((tMaxZ * tMaxZ) > rad) break;
                z += stepZ;
                tMaxZ += tDeltaZ;
                face.vx = 0;
                face.vy = 0;
                face.vz = -stepZ;
            }
        } else {
            if (tMaxY < tMaxZ) {
                if ((tMaxY * tMaxY) > rad) break;
                y += stepY;
                tMaxY += tDeltaY;
                face.vx = 0;
                face.vy = -stepY;
                face.vz = 0;
            } else {
                // Identical to the second case, repeated for simplicity in
                // the conditionals.
                if ((tMaxZ * tMaxZ) > rad) break;
                z += stepZ;
                tMaxZ += tDeltaZ;
                face.vx = 0;
                face.vy = 0;
                face.vz = -stepZ;
            }
        }
    }
    // DEBUG_LOG("Raycast failed\n");
    return (RayCastResult) {
        .pos = vec3_i32_all(0),
        .block = NULL,
        .face = vec3_i32_all(0)
    };
}
