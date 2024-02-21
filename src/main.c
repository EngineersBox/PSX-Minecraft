#include <stdint.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "../structure/primitive/clip.h"
#include "render/render_context.h"
#include "resources/assets.h"
#include "core/input.h"
#include "core/camera.h"
#include "structure/primitive/cube.h"
#include "world/world.h"
#include "util/math_utils.h"
#include "render/debug.h"
#include "ui/crosshair.h"
#include "ui/axis.h"

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
Transforms transforms = {};

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
    // Create transforms
    transforms = (Transforms) {
        .translation_rotation = {0},
        .translation_position = {0, 0, 0},
        .geometry_mtx = {},
        .lighting_mtx = light_mtx
    };
    // Load font and open a text stream
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 100);
    // Unpack LZP archive and load assets
    assetsLoad();
}

#define MARKER_SIZE 10
SVECTOR verts[] = {
    { -MARKER_SIZE, -MARKER_SIZE, -MARKER_SIZE, 0 },
    {  MARKER_SIZE, -MARKER_SIZE, -MARKER_SIZE, 0 },
    { -MARKER_SIZE,  MARKER_SIZE, -MARKER_SIZE, 0 },
    {  MARKER_SIZE,  MARKER_SIZE, -MARKER_SIZE, 0 },
    {  MARKER_SIZE, -MARKER_SIZE,  MARKER_SIZE, 0 },
    { -MARKER_SIZE, -MARKER_SIZE,  MARKER_SIZE, 0 },
    {  MARKER_SIZE,  MARKER_SIZE,  MARKER_SIZE, 0 },
    { -MARKER_SIZE,  MARKER_SIZE,  MARKER_SIZE, 0 }
};

bool render_marker = false;
VECTOR marker_pos = {0};
SVECTOR marker_rot = {0};
SVECTOR marker_verts[8];

void cameraStartHandler(Camera* camera) {
    result = worldRayCastIntersection(&world, camera, ONE * 5);
    result.pos.vx >>= FIXED_POINT_SHIFT;
    result.pos.vy >>= FIXED_POINT_SHIFT;
    result.pos.vz >>= FIXED_POINT_SHIFT;
    printf(
        "Ray cast result: [Pos: (%d,%d,%d)] [Block: %d] [Face: (%d,%d,%d)]\n",
        result.pos.vx,
        result.pos.vy,
        result.pos.vz,
        result.block,
        result.face.vx,
        result.face.vy,
        result.face.vz
    );
    if (blockIsOpaque(result.block)) {
        marker_pos.vx = (result.pos.vx * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
        marker_pos.vy = (-result.pos.vy * BLOCK_SIZE) - (BLOCK_SIZE >> 1);
        marker_pos.vz = (result.pos.vz * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
        worldModifyVoxel(&world, &result.pos, BLOCKID_AIR);
        printf(
            "Marker: (%d,%d,%d) Camera: (%d,%d,%d)\n",
            marker_pos.vx,
            marker_pos.vy,
            marker_pos.vz,
            camera->position.vx,
            camera->position.vy,
            camera->position.vz
        );
#define createVert(i) marker_verts[i] = (SVECTOR) { marker_pos.vx + verts[i].vx, marker_pos.vy + verts[i].vy, marker_pos.vz + verts[i].vz, 0 }
        createVert(0);
        createVert(1);
        createVert(2);
        createVert(3);
        createVert(4);
        createVert(5);
        createVert(6);
        createVert(7);
        render_marker = true;
    } else {
        render_marker = false;
    }
}

void drawMarker() {
    if (!render_marker) {
        return;
    }
    marker_rot.vy += 16;
    marker_rot.vz += 16;
    POLY_F4* pol4;
    int p, sz;
    SVECTOR spos;
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&marker_rot, &omtx);
    TransMatrix(&omtx, &marker_pos);
    // Multiply light matrix to object matrix
    MulMatrix0(&transforms.lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&transforms.geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    for (int i=0; i<8; i++) {
        pol4 = (POLY_F4*) allocatePrimitive(&render_context, sizeof(POLY_F4));
        gte_ldv3(
            &verts[CUBE_INDICES[i].v0],
            &verts[CUBE_INDICES[i].v1],
            &verts[CUBE_INDICES[i].v2]
        );
        // Rotation, Translation and Perspective Triple
        gte_rtpt();
        gte_nclip();
        gte_stopz(&p);
        if (p < 0) {
            freePrimitive(&render_context, sizeof(POLY_F4));
            // printf("Behind\n");
            continue;
        }
        // Average screen Z result for four primtives
        gte_avsz4();
        gte_stotz(&p);
        // Initialize a textured quad primitive
        setPolyF4(pol4);
        // Set the projected vertices to the primitive
        gte_stsxy0(&pol4->x0);
        gte_stsxy1(&pol4->x1);
        gte_stsxy2(&pol4->x2);
        // Compute the last vertex and set the result
        gte_ldv0(&verts[CUBE_INDICES[i].v3]);
        gte_rtps();
        gte_stsxy(&pol4->x3);
        // Test if quad is off-screen, discard if so
        // if (quadClip(
        //     &render_context.screen_clip,
        //     (DVECTOR *) &pol4->x0,
        //     (DVECTOR *) &pol4->x1,
        //     (DVECTOR *) &pol4->x2,
        //     (DVECTOR *) &pol4->x3)) {
        //     freePrimitive(&render_context, sizeof(POLY_F4));
        //     printf("Clipped\n");
        //     continue;
        // }
        setRGB0(
            pol4,
            0xff,
            0x0,
            0x0
        );
        gte_ldrgb(&pol4->r0);
        // Load the face normal
        gte_ldv0(&CUBE_NORMS[i]);
        // Normal Color Column Single
        gte_nccs();
        // Store result to the primitive
        gte_strgb(&pol4->r0);
        uint32_t* ot_entry = allocateOrderingTable(&render_context, 0);
        addPrim(ot_entry, pol4);
    }
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
    worldInit(&world, &render_context);
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
        // Draw marker
        drawMarker();
        // Render UI
        // crosshairDraw(&render_context);
        axisDraw(&render_context, &transforms, &camera);
        debugDrawPBUsageGraph(&render_context, 0, SCREEN_YRES);
        // Flush font to screen
        FntFlush(0);
        // Swap buffers and draw the primitives
        swapBuffers(&render_context);
    }
    // chunkDestroy(&chunk);
    worldDestroy(&world);
    assetsFree();
    return 0;
}
