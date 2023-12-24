#include "cube.h"

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "../core/display.h"

const SVECTOR CUBE_NORMS[6] = {
    {0, 0, -ONE, 0},
    {0, 0, ONE, 0},
    {0, -ONE, 0, 0},
    {0, ONE, 0, 0},
    {-ONE, 0, 0, 0},
    {ONE, 0, 0, 0}
};

const INDEX CUBE_INDICES[6] = {
    {0, 1, 2, 3},
    {4, 5, 6, 7},
    {5, 4, 0, 1},
    {6, 7, 3, 2},
    {0, 2, 5, 7},
    {3, 1, 6, 4}
};

void cubeRender(DisplayContext *ctx, Cube *cube) {
    int p;
    POLY_FT4* pol4 = (POLY_FT4*) ctx->primitive;;
    for (int i = 0; i < CUBE_FACES; i++) {
        /* Load the first 3 vertices of a quad to the GTE */
        gte_ldv3(
            &cube->vertices[CUBE_INDICES[i].v0],
            &cube->vertices[CUBE_INDICES[i].v1],
            &cube->vertices[CUBE_INDICES[i].v2]
        );
        /* Rotation, Translation and Perspective Triple */
        gte_rtpt();
        /* Compute normal clip for backface culling */
        gte_nclip();
        /* Get result*/
        gte_stopz(&p);
        /* Skip this face if backfaced */
        if (p < 0) continue;
        /* Calculate average Z for depth sorting */
        gte_avsz4();
        gte_stotz(&p);
        /* Skip if clipping off */
        /* (the shift right operator is to scale the depth precision) */
        if ((p >> 2) > ORDERING_TABLE_LENGTH) continue;
        /* Initialize a quad primitive */
        setPolyFT4(pol4);
        /* Set the projected vertices to the primitive */
        gte_stsxy0(&pol4->x0);
        gte_stsxy1(&pol4->x1);
        gte_stsxy2(&pol4->x2);
        /* Compute the last vertex and set the result */
        gte_ldv0(&cube->vertices[CUBE_INDICES[i].v3]);
        gte_rtps();
        gte_stsxy(&pol4->x3);
        /* Load primitive color even though gte_ncs() doesn't use it. */
        /* This is so the GTE will output a color result with the */
        /* correct primitive code. */
        gte_ldrgb(&pol4->r0);
        /* Load the face normal */
        gte_ldv0(&CUBE_NORMS[i]);
        /* Normal Color Single */
        gte_ncs();
        /* Store result to the primitive */
        gte_strgb(&pol4->r0);
        /* Set face texture */
        setUVWH(pol4, 0, 1, 128, 128);
        pol4->tpage = cube->texture_tpage;
        pol4->clut = cube->texture_clut;
        /* Sort primitive to the ordering table */
        addPrim(ctx->db[ctx->active].ordering_table + (p >> 2), pol4);
        /* Advance to make another primitive */
        pol4++;
    }
    ctx->primitive = (char*) pol4;
}
