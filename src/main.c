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

/* Cube vertices */
SVECTOR _cube_verts[] = {
    { -100, -100, -100, 0 },
    {  100, -100, -100, 0 },
    { -100,  100, -100, 0 },
    {  100,  100, -100, 0 },
    {  100, -100,  100, 0 },
    { -100, -100,  100, 0 },
    {  100,  100,  100, 0 },
    { -100,  100,  100, 0 }
};

/* Cube face normals */
SVECTOR _cube_norms[] = {
    { 0, 0, -ONE, 0 },
    { 0, 0, ONE, 0 },
    { 0, -ONE, 0, 0 },
    { 0, ONE, 0, 0 },
    { -ONE, 0, 0, 0 },
    { ONE, 0, 0, 0 }
};

/* Cube vertex indices */
INDEX _cube_indices[] = {
    { 0, 1, 2, 3 },
    { 4, 5, 6, 7 },
    { 5, 4, 0, 1 },
    { 6, 7, 3, 2 },
    { 0, 2, 5, 7 },
    { 3, 1, 6, 4 }
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
        pol4 = (POLY_FT4*) dctx.primitive;
		int p;
        for(int i = 0; i < CUBE_FACES; i++) {

            /* Load the first 3 vertices of a quad to the GTE */
            gte_ldv3(
                &_cube_verts[_cube_indices[i].v0],
                &_cube_verts[_cube_indices[i].v1],
                &_cube_verts[_cube_indices[i].v2] );

            /* Rotation, Translation and Perspective Triple */
            gte_rtpt();

            /* Compute normal clip for backface culling */
            gte_nclip();

            /* Get result*/
            gte_stopz( &p );
            /* Skip this face if backfaced */
            if( p < 0 )continue;

            /* Calculate average Z for depth sorting */
            gte_avsz4();
            gte_stotz( &p );

            /* Skip if clipping off */
            /* (the shift right operator is to scale the depth precision) */
            if( (p>>2) > ORDERING_TABLE_LENGTH )continue;

            /* Initialize a quad primitive */
            setPolyFT4( pol4 );

            /* Set the projected vertices to the primitive */
            gte_stsxy0( &pol4->x0 );
            gte_stsxy1( &pol4->x1 );
            gte_stsxy2( &pol4->x2 );

            /* Compute the last vertex and set the result */
            gte_ldv0( &_cube_verts[_cube_indices[i].v3] );
            gte_rtps();
            gte_stsxy( &pol4->x3 );

            /* Load primitive color even though gte_ncs() doesn't use it. */
            /* This is so the GTE will output a color result with the */
            /* correct primitive code. */
            gte_ldrgb( &pol4->r0 );

            /* Load the face normal */
            gte_ldv0( &_cube_norms[i] );
            /* Normal Color Single */
            gte_ncs();
            /* Store result to the primitive */
            gte_strgb( &pol4->r0 );
            /* Set face texture */
            setUVWH( pol4, 0, 1, 128, 128 );
            pol4->tpage = texture_tpage;
            pol4->clut = texture_clut;

            /* Sort primitive to the ordering table */
            addPrim(dctx.db[dctx.active].ordering_table + (p>>2), pol4);
            /* Advance to make another primitive */
            pol4++;
        }
        // cubeRender(&dctx, &cube);
        /* Update nextpri variable */
        /* (IMPORTANT if you plan to sort more primitives after this) */
        dctx.primitive = (char*) pol4;
        /* Swap buffers and draw the primitives */
        display(&dctx);
    }
    return 0;
}
