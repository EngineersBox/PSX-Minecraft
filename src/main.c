#include <display.h>
#include <stdint.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "core/display.h"
#include "primitive/primitive.h"
#include "assets.h"
#include "blocks/cube.h"

DisplayContext dctx = {
    .active = 0,
    .db = {},
    .primitive = NULL
};

/* Light color matrix */
/* Each column represents the color matrix of each light source and is */
/* used as material color when using gte_ncs() or multiplied by a */
/* source color when using gte_nccs(). 4096 is 1.0 in this matrix */
/* A column of zeroes disables the light source. */
MATRIX color_mtx = {
    ONE * 3 / 4, 0, 0, /* Red   */
    ONE * 3 / 4, 0, 0, /* Green */
    ONE * 3 / 4, 0, 0 /* Blue  */
};

/* Light matrix */
/* Each row represents a vector direction of each light source. */
/* An entire row of zeroes disables the light source. */
MATRIX light_mtx = {
    /* X,  Y,  Z */
    -2048, -2048, -2048,
    0, 0, 0,
    0, 0, 0
};

/* Reference texture data */
extern const uint32_t tim_texture[];

/* TPage and CLUT values */
uint16_t texture_tpage; /* For the scrolling blending pattern */
uint16_t texture_clut;

/* Function declarations */
void init() {
    TIM_IMAGE tim;
    initDisplay(&dctx);
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

/* Main function */
int main() {
    SVECTOR rot = {0}; /* Rotation vector for Rotmatrix */
    VECTOR pos = {0, 0, 400}; /* Translation vector for TransMatrix */
    MATRIX mtx, lmtx; /* Rotation matrices for geometry and lighting */
    POLY_FT4* pol4; /* Flat shaded textured quad primitive pointer */
    /* Init graphics and GTE */
    init();
    Cube cube = {
        .vertices = {
            {-100, -100, -100, 0},
            {100, -100, -100, 0},
            {-100, 100, -100, 0},
            {100, 100, -100, 0},
            {100, -100, 100, 0},
            {-100, -100, 100, 0},
            {100, 100, 100, 0},
            {-100, 100, 100, 0}
        },
        .texture_tpage = texture_tpage,
        .texture_clut = texture_clut
    };
    /* Main loop */
    while (1) {
        /* Set rotation and translation to the matrix */
        RotMatrix(&rot, &mtx);
        TransMatrix(&mtx, &pos);
        /* Multiply light matrix by rotation matrix so light source */
        /* won't appear relative to the model's rotation */
        MulMatrix0(&light_mtx, &mtx, &lmtx);
        /* Set rotation, translation and light matrices */
        gte_SetRotMatrix(&mtx);
        gte_SetTransMatrix(&mtx);
        gte_SetLightMatrix(&lmtx);
        /* Make the cube SPEEN */
        rot.vx += 16;
        rot.vz += 16;
        /* Draw the cube */
        // pol4 = (POLY_FT4*) dctx.primitive;
        cubeRender(&dctx, &cube);
        /* Update nextpri variable */
        /* (IMPORTANT if you plan to sort more primitives after this) */
        // dctx.primitive = (char*) pol4;
        /* Swap buffers and draw the primitives */
        display(&dctx);
    }
    return 0;
}
