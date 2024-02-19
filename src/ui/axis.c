#include "axis.h"

#include <inline_c.h>
#include <psxgte.h>
#include <psxgpu.h>

#include "../util/math_utils.h"

const SVECTOR origin = (SVECTOR) {
    .vx = 0,
    .vy = 0,
    .vz = 0
};
const SVECTOR x_direction = (SVECTOR) {
    .vx = 3,
    .vy = 0,
    .vz = 0
};
const SVECTOR y_direction = (SVECTOR) {
    .vx = 0,
    .vy = -3,
    .vz = 0
};
const SVECTOR z_direction = (SVECTOR) {
    .vx = 0,
    .vy = 0,
    .vz = 3
};

void drawLine(RenderContext* ctx, const SVECTOR* vertex, uint8_t r, uint8_t g, uint8_t b) {
    LINE_F2* line = (LINE_F2*) allocatePrimitive(ctx, sizeof(LINE_F2));
    gte_ldv01(&origin, vertex);
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    // Initialize a line
    setLineF2(line);
    // Set the projected vertices to the primitive
    gte_stsxy0(&line->x0);
    gte_stsxy1(&line->x1);
    setRGB0(line, r, g, b);
    uint32_t* ot_object = allocateOrderingTable(ctx, 0);
    addPrim(ot_object, line);
}

void axisDraw(RenderContext* ctx, const Transforms* transforms, const Camera* camera) {
    static SVECTOR rotation = {0};
    VECTOR position = (VECTOR) {
        .vx = camera->position.vx >> FIXED_POINT_SHIFT,
        .vy = camera->position.vy >> FIXED_POINT_SHIFT,
        .vz = camera->position.vz >> FIXED_POINT_SHIFT
    };
    // Object and light matrix for object
    MATRIX omtx;
    // Set object rotation and position
    RotMatrix(&rotation, &omtx);
    TransMatrix(&omtx, &position);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV((MATRIX*) &transforms->geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    // gte_SetTransMatrix(&omtx);
    drawLine(ctx, &x_direction, 0xff, 0x0, 0x0);
    drawLine(ctx, &y_direction, 0x0, 0xff, 0x0);
    drawLine(ctx, &z_direction, 0x0, 0x0, 0xff);
    // Restore matrix
    PopMatrix();
}
