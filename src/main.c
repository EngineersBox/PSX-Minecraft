#include <display.h>
#include <stdint.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "core/display.h"
#include "primitive/primitive.h"
#include "assets.h"
#include "primitive/clip.h"
#include "core/input.h"
#include "core/camera.h"
#include "blocks/cube.h"

DisplayContext dctx = {
    .active = 0,
    .db = {},
    .primitive = NULL
};
Input input = {};

// Light color matrix
// Each column represents the color matrix of each light source and is
// used as material color when using gte_ncs() or multiplied by a
// source color when using gte_nccs(). 4096 is 1.0 in this matrix
// A column of zeroes effectively disables the light source.
MATRIX color_mtx = {
    ONE, 0, 0,	// Red
    0, 0, 0,	// Green
    ONE, 0, 0	// Blue
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX light_mtx = {
    /* X,  Y,  Z */
    -2048 , -2048 , -2048,
    0	  , 0	  , 0,
    0	  , 0	  , 0
};

// Reference texture data
extern const uint32_t tim_texture[];

uint16_t texture_tpage; // Scrolling + blending pattern
uint16_t texture_clut;


void init() {
    TIM_IMAGE tim;
    initDisplay(&dctx);
    initInput(&input);
    /* Set light ambient color and light color matrix */
    gte_SetBackColor(63, 63, 63);
    gte_SetColorMatrix(&color_mtx);
    /* Load .TIM file */
    GetTimInfo(tim_texture, &tim);
    if (tim.mode & 0x8) {
        LoadImage(tim.crect, tim.caddr); /* Upload CLUT if present */
    }
    LoadImage(tim.prect, tim.paddr); /* Upload texture to VRAM */
    texture_tpage = getTPage(tim.mode, 1, tim.prect->x, tim.prect->y);
    texture_clut = getClut(tim.crect->x, tim.crect->y);
}

void floorRender(SVECTOR floor_verts[17][17]) {
    int p;
    // Draw the floor
    POLY_F4* pol4 = (POLY_F4*) dctx.primitive;

    for(int py = 0; py < 16; py++) {
        for(int px = 0; px < 16; px++) {
            // Load first three vertices to GTE
            gte_ldv3(
                &floor_verts[py][px],
                &floor_verts[py][px+1],
                &floor_verts[py+1][px]
            );
            gte_rtpt();
            gte_avsz3();
            gte_stotz(&p);
            if (((p >> 2) >= ORDERING_TABLE_LENGTH) || ((p >> 2) <= 0)) continue;
            setPolyF4(pol4);
            // Set the projected vertices to the primitive
            gte_stsxy0(&pol4->x0);
            gte_stsxy1(&pol4->x1);
            gte_stsxy2(&pol4->x2);
            // Compute the last vertex and set the result
            gte_ldv0(&floor_verts[py+1][px+1]);
            gte_rtps();
            gte_stsxy(&pol4->x3);
            // Test if quad is off-screen, discard if so
            // Clipping is important as it not only prevents primitive
            // overflows (tends to happen on textured polys) but also
            // saves packet buffer space and speeds up rendering.
            if (quad_clip(
                &dctx.screen_clip,
                (DVECTOR*)&pol4->x0,
                (DVECTOR*)&pol4->x1,
                (DVECTOR*)&pol4->x2,
                (DVECTOR*)&pol4->x3)) {
                continue;
            }
            gte_avsz4();
            gte_stotz(&p);
            if ((px+py)&0x1) {
                setRGB0(pol4, 128, 128, 128);
            } else {
                setRGB0(pol4, 255, 255, 255);
            }
            addPrim(dctx.db[dctx.active].ordering_table + (p >> 2), pol4);
            pol4++;
        }
    }
    // Update nextpri variable (very important)
    dctx.primitive = (char*)pol4;
}


int main() {
    Camera camera = {
        .position = {0, ONE * -200, 0},
        .rotation = {0, 0, 0},
        .mode = 0
    };
    Transforms transforms = {
        .translation_rotation = {0},
        .translation_position = {0, 0, 200},
        .geometry_mtx = {},
        .lighting_mtx = {}
    };
    SVECTOR floor_verts[17][17]; // Floor vertices
    POLY_FT4* pol4;
    init();
    Cube cube = {
        .position = {0, 0, 200},
        .rotation = {0, 0, 0},
        .vertices = {
            {-20, -20, -20, 0},
            {20, -20, -20, 0},
            {-20, 20, -20, 0},
            {20, 20, -20, 0},
            {20, -20, 20, 0},
            {-20, -20, 20, 0},
            {20, 20, 20, 0},
            {-20, 20, 20, 0}
        },
        .texture_tpage = texture_tpage,
        .texture_clut = texture_clut
    };
    /* Main loop */
    while (1) {
        camera.mode = 0;
        cameraUpdate(&camera, &input, &transforms, &cube.position);
        // Set rotation and translation matrix
        gte_SetRotMatrix(&transforms.geometry_mtx);
        gte_SetTransMatrix(&transforms.geometry_mtx);
        // Render floor
        floorRender(floor_verts);
        // Draw the cube
        cubeRender(&dctx, &transforms, &cube);
        // Swap buffers and draw the primitives
        display(&dctx);
    }
    return 0;
}
