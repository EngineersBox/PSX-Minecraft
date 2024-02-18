#include <clip.h>
#include <stdint.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "render/render_context.h"
#include "resources/assets.h"
#include "core/input.h"
#include "core/camera.h"
#include "structure/primitive/cube.h"
#include "world/world.h"
#include "util/math_utils.h"
#include "render/debug.h"

RenderContext render_context = {
    .active = 0,
    .db = {},
    .primitive = NULL
};
Input input = {};
// Reference texture data
extern const uint32_t tim_texture[];
World world = {};
RayCastResult result = {};

// Light color matrix
// Each column represents the color matrix of each light source and is
// used as material color when using gte_ncs() or multiplied by a
// source color when using gte_nccs(). 4096 is 1.0 in this matrix
// A column of zeroes effectively disables the light source.
MATRIX color_mtx = {
    .m = {
        {ONE * 3 / 4, 0, 0}, /* Red   */
        {ONE * 3 / 4, 0, 0}, /* Green */
        {ONE * 3 / 4, 0, 0} /* Blue  */
    }
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX light_mtx = {
    /* X,  Y,  Z */
    .m = {
        {-FIXED_1_2, -FIXED_1_2, FIXED_1_2},
        {0, 0, 0},
        {0, 0, 0}
    }
};

void init() {
    blockInitialiseBuiltin();
    initRenderContext(&render_context);
    initInput(&input);
    /* Set light ambient color and light color matrix */
    gte_SetBackColor(63, 63, 63);
    gte_SetColorMatrix(&color_mtx);
    // Load font and open a text stream
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 100);
    // Unpack LZP archive and load assets
    assetsLoad();
}

void cameraStartHandler(Camera* camera) {
    result = worldRayCastIntersection(&world, camera, ONE * 5);
    printf(
        "Ray cast result: [Pos: (%d,%d,%d)] [Block: %d] [Face: (%d,%d,%d)]\n",
        result.pos.vx >> FIXED_POINT_SHIFT,
        result.pos.vy >> FIXED_POINT_SHIFT,
        result.pos.vz >> FIXED_POINT_SHIFT,
        result.block,
        result.face.vx,
        result.face.vy,
        result.face.vz
    );
}

// TODO: Move crosshair to UI handler and structs
void drawCrossHair() {
    LINE_F2* vertical = (LINE_F2*) allocatePrimitive(&render_context, sizeof(LINE_F2));
    setXY2(
        vertical,
        CENTRE_X, CENTRE_Y - 2,
        CENTRE_X, CENTRE_Y + 2
    );
    setRGB0(vertical, 0xff, 0xff, 0xff);
    LINE_F2* horizontal = (LINE_F2*) allocatePrimitive(&render_context, sizeof(LINE_F2));
    setXY2(
        horizontal,
        CENTRE_X - 2, CENTRE_Y,
        CENTRE_X + 2, CENTRE_Y
    );
    setRGB0(horizontal, 0xff, 0xff, 0xff);
    lineF2Render(vertical, 0, &render_context);
    lineF2Render(horizontal, 0, &render_context);
}

SMD_PRIM x_prim = (SMD_PRIM) {
    .prim_id = 0,
    .r0 = 0xff,
    .g0 = 0x1,
    .b0 = 0x1,
};
SVECTOR x_vert[2] = {
    (SVECTOR) {
        .vx = 0,
        .vy = 0,
        .vz = 0
    },
    (SVECTOR) {
        .vx = 3,
        .vy = 0,
        .vz = 0
    }
};

SMD_PRIM y_prim = (SMD_PRIM) {
    .prim_id = 0,
    .r0 = 0x1,
    .g0 = 0x1,
    .b0 = 0xff,
};
SVECTOR y_vert[2] = {
    (SVECTOR) {
        .vx = 0,
        .vy = 0,
        .vz = 0
    },
    (SVECTOR) {
        .vx = 0,
        .vy = 3,
        .vz = 0
    }
};

SMD_PRIM z_prim = (SMD_PRIM) {
    .prim_id = 0,
    .r0 = 0x1,
    .g0 = 0xff,
    .b0 = 0x1,
};
SVECTOR z_vert[2] = {
    (SVECTOR) {
        .vx = 0,
        .vy = 0,
        .vz = 0
    },
    (SVECTOR) {
        .vx = 0,
        .vy = 0,
        .vz = 3
    }
};

void drawLine(SMD_PRIM* primitive, SVECTOR vertices[2]) {
    LINE_F2* line = (LINE_F2*) allocatePrimitive(&render_context, sizeof(LINE_F2));
    gte_ldv01(
        &vertices[0],
        &vertices[1]
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    // Initialize a line
    setLineF2(line);
    // Set the projected vertices to the primitive
    gte_stsxy0(&line->x0);
    gte_stsxy1(&line->x1);
    setRGB0(
        line,
        primitive->r0,
        primitive->g0,
        primitive->b0
    );
    uint32_t* ot_object = allocateOrderingTable(&render_context, 0);
    addPrim(ot_object, line);
}

// TODO: Move this to UI directory with dependent structure
void drawAxis(Camera* camera, Transforms* transforms) {
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
    CompMatrixLV(&transforms->geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    // gte_SetTransMatrix(&omtx);
    drawLine(&x_prim, x_vert);
    drawLine(&y_prim, y_vert);
    drawLine(&z_prim, z_vert);
    // Restore matrix
    PopMatrix();
}

int main() {
    init();
    Camera camera = {
        .position = { ONE * 0, ONE * 0, ONE * 0 },
        .rotation = {ONE * 248, ONE * -1592, 0},
        .mode = 0,
        .start_handler = &cameraStartHandler
    };
    Transforms transforms = {
        .translation_rotation = {0},
        .translation_position = {0, 0, 0},
        .geometry_mtx = {},
        .lighting_mtx = light_mtx
    };
    world = (World) {
        .head = {
            .vx = 0,
            .vz = 0
        },
        .centre = {
            .vx = 0,
            .vy = 0,
            .vz = 0
        }
    };
    worldInit(&world);
    while (1) {
        // Set pad pointer to buffer data
        camera.mode = 0;
        cameraUpdate(&camera, &input, &transforms, &result.pos);
        worldUpdate(&world, &camera.position);
        // Set rotation and translation matrix
        gte_SetRotMatrix(&transforms.geometry_mtx);
        gte_SetTransMatrix(&transforms.geometry_mtx);
        // Draw the world
        worldRender(&world, &render_context, &transforms);
        // Clear window constraints
        renderClearConstraints(&render_context);
        // Flush font to screen
        FntFlush(-1);
        // testLine();
        // drawCrossHair();
        drawAxis(&camera, &transforms);
        debugDrawPBUsageGraph(&render_context, 0, SCREEN_YRES);
        // Swap buffers and draw the primitives
        swapBuffers(&render_context);
    }
    // chunkDestroy(&chunk);
    worldDestroy(&world);
    assetsFree();
    return 0;
}
