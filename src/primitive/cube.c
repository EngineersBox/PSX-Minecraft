#include "cube.h"

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>
#include <stdlib.h>

#include "../core/display.h"
#include "clip.h"

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
    {5, 0, 7, 2},
    {1, 4, 3, 6}
};

// #define RENDER_SIDES_COLOURED

void cubeRender(Cube *cube, DisplayContext *ctx, Transforms *transforms) {
    int p;
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&cube->rotation, &omtx);
    TransMatrix(&omtx, &cube->position);
    // Multiply light matrix to object matrix
    MulMatrix0(&transforms->lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&transforms->geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    // Sort the cube
    POLY_FT4* pol4 = (POLY_FT4*) ctx->primitive;
    for (int i = 0; i < CUBE_FACES; i++) {
        gte_ldv3(
            &cube->vertices[CUBE_INDICES[i].v0],
            &cube->vertices[CUBE_INDICES[i].v1],
            &cube->vertices[CUBE_INDICES[i].v2]
        );
        // Rotation, Translation and Perspective Triple
        gte_rtpt();
        gte_nclip();
        gte_stopz(&p);
        if (p < 0) continue;
        // Average screen Z result for four primtives
        gte_avsz4();
        gte_stotz(&p);
        // (the shift right operator is to scale the depth precision)
        if (((p >> 2) <= 0) || ((p >> 2) >= ORDERING_TABLE_LENGTH)) continue;
        // Initialize a textured quad primitive
        setPolyFT4(pol4);
        // Set the projected vertices to the primitive
        gte_stsxy0(&pol4->x0);
        gte_stsxy1(&pol4->x1);
        gte_stsxy2(&pol4->x2);
        // Compute the last vertex and set the result
        gte_ldv0(&cube->vertices[CUBE_INDICES[i].v3]);
        gte_rtps();
        gte_stsxy(&pol4->x3);
        // Test if quad is off-screen, discard if so
        if (quad_clip(
            &ctx->screen_clip,
            (DVECTOR *) &pol4->x0,
            (DVECTOR *) &pol4->x1,
            (DVECTOR *) &pol4->x2,
            (DVECTOR *) &pol4->x3)) {
            continue;
        }
        // Load primitive color even though gte_ncs() doesn't use it.
        // This is so the GTE will output a color result with the
        // correct primitive code.
        const TextureAttributes attributes = cube->texture_face_attrib[i];
        if (attributes.tint.cd) {
            setRGB0(
                pol4,
                attributes.tint.r,
                attributes.tint.g,
                attributes.tint.b
            );
        }
        gte_ldrgb(&pol4->r0);
        // Load the face normal
        gte_ldv0(&CUBE_NORMS[i]);
        // Apply RGB tinting to lighting calculation result on the basis
        // that it is enabled. This corresponds to the column based calc
        if (attributes.tint.cd) {
            // Normal Color Column Single
            gte_nccs();
        } else {
            // Normal Color Single
            gte_ncs();
        }
        // Store result to the primitive
        gte_strgb(&pol4->r0);
        // Set texture coords and dimensions
        setUVWH(
            pol4,
            attributes.u,
            attributes.v,
            attributes.w,
            attributes.h
        );
        // Bind texture page and colour look-up-table
        pol4->tpage = textures[cube->texture].tpage;
        pol4->clut = textures[cube->texture].clut;
        // Sort primitive to the ordering table
        addPrim(ctx->db[ctx->active].ordering_table + (p >> 2), pol4);
        // Advance to make another primitive
        pol4++;
    }
    // Update nextpri
    ctx->primitive = (char *) pol4;
    // Restore matrix
    PopMatrix();
}
