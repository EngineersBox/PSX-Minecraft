#include <cvector_utils.h>
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
#include "structure/cvector.h"

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
cvector(SVECTOR) markers = NULL;
VECTOR zero_vec = {0};
SVECTOR zero_svec = {0};

SVECTOR origin_pos = {0};
SVECTOR marker_pos = {0};
SVECTOR marker_rot = {0};

void cameraStartHandler(Camera* camera) {
    cvector_clear(markers);
    result = worldRayCastIntersection(&world, camera, ONE * 5, &markers);
    printf("Marker count: %d\n", cvector_size(markers));
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
    origin_pos.vx = (((camera->position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    origin_pos.vy = (((camera->position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    origin_pos.vz = (((camera->position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    marker_pos.vx =  (result.pos.vx * BLOCK_SIZE) + (BLOCK_SIZE >> 1); // + ((result.face.vx >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    marker_pos.vy = (-result.pos.vy * BLOCK_SIZE) - (BLOCK_SIZE >> 1); // + ((result.face.vy >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    marker_pos.vz =  (result.pos.vz * BLOCK_SIZE) + (BLOCK_SIZE >> 1); // + ((result.face.vz >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
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
    render_marker = true;
    SVECTOR* cmarker;
    cvector_for_each_in(cmarker, markers) {
        printf("[TRACE] MARKER: (%d,%d,%d)\n", inlineVecPtr(cmarker));
    }
}

void drawRayLine() {
    MATRIX omtx, olmtx;
    // Ray trace line
    RotMatrix(&zero_svec, &omtx);
    TransMatrix(&omtx, &zero_vec);
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
    // Generate line
    LINE_F2* line = (LINE_F2*) allocatePrimitive(&render_context, sizeof(LINE_F2));
    setLineF2(line);
    gte_ldv0(&origin_pos);
    // Rotation, Translation and Perspective Single
    gte_rtps();
    gte_stsxy(&line->x0);
    gte_ldv0(&marker_pos);
    // Rotation, Translation and Perspective Single
    gte_rtps();
    gte_stsxy(&line->x1);
    setRGB0(
        line,
        0x0,
        0x0,
        0xff
    );
    uint32_t* ot_entry = allocateOrderingTable(&render_context, 0);
    addPrim(ot_entry, line);
    PopMatrix();
}

void drawMarker() {
    if (!render_marker) {
        return;
    }
    VECTOR zero = {0};
    drawRayLine();
    // Trace end marker
    // marker_rot.vy += 16;
    // marker_rot.vz += 16;
    POLY_F4* pol4;
    int p;
    MATRIX omtx, olmtx;
    // printf("CURRENT MARKER: (%d,%d,%d)\n", inlineVecPtr(current_marker));
    // Set object rotation and position
    RotMatrix(&marker_rot, &omtx);
    TransMatrix(&omtx, &zero);
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
    cvector_iterator(SVECTOR) current_marker;
    cvector_for_each_in(current_marker, markers) {
        // Set matrices
        gte_SetRotMatrix(&omtx);
        gte_SetTransMatrix(&omtx);
        for (int i=0; i<8; i++) {
            pol4 = (POLY_F4*) allocatePrimitive(&render_context, sizeof(POLY_F4));
#define createVert(_v) (SVECTOR) { \
            current_marker->vx + verts[CUBE_INDICES[i]._v].vx, \
            current_marker->vy + verts[CUBE_INDICES[i]._v].vy, \
            current_marker->vz + verts[CUBE_INDICES[i]._v].vz, \
            0 \
        }
            SVECTOR current_verts[4] = {
                createVert(v0),
                createVert(v1),
                createVert(v2),
                createVert(v3)
            };
            gte_ldv3(
                &current_verts[0],
                &current_verts[1],
                &current_verts[2]
            );
            // Rotation, Translation and Perspective Triple
            gte_rtpt();
            gte_nclip();
            gte_stopz(&p);
            if (p < 0) {
                freePrimitive(&render_context, sizeof(POLY_F4));
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
            gte_ldv0(&current_verts[3]);
            gte_rtps();
            gte_stsxy(&pol4->x3);
            // Test if quad is off-screen, discard if so
            if (quadClip(
                &render_context.screen_clip,
                (DVECTOR*) &pol4->x0,
                (DVECTOR*) &pol4->x1,
                (DVECTOR*) &pol4->x2,
                (DVECTOR*) &pol4->x3)) {
                freePrimitive(&render_context, sizeof(POLY_F4));
                continue;
                }
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
    }
    PopMatrix();
}

int main() {
    cvector_init(markers, 1, NULL);
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
