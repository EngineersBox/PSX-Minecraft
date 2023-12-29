#include "chunk_mesh.h"

#include <clip.h>
#include <inline_c.h>
#include <stdlib.h>

void __primtiveDestructor(void *elem) {
    memset(elem, 0, sizeof(SMD_PRIM));
}

void __svectorDestructor(void *elem) {
    memset(elem, 0, sizeof(SVECTOR));
}

void chunkMeshInit(ChunkMesh *mesh) {
    // !IMPORTANT: This null init is important for cvector to ensure allocation is done initially
    mesh->primitives = NULL;
    mesh->vertices = NULL;
    mesh->normals = NULL;
    cvector_init(mesh->primitives, 1, __primtiveDestructor);
    cvector_init(mesh->vertices, 1, __svectorDestructor);
    cvector_init(mesh->normals, 1, __svectorDestructor);
}

void chunkMeshDestroy(const ChunkMesh *mesh) {
    cvector_free(mesh->primitives);
    cvector_free(mesh->vertices);
    cvector_free(mesh->normals);
}

void chunkMeshClear(ChunkMesh *mesh) {
    cvector_clear(mesh->primitives);
    cvector_clear(mesh->vertices);
    cvector_clear(mesh->normals);
    mesh->smd = (SMD){};
    memset(&mesh->smd, 0, sizeof(SMD));
}

void renderLine(SMD_PRIM *primitive, DisplayContext *ctx, Transforms *transforms) {
    // TODO
}

void renderTriangle(SMD_PRIM *primitive, DisplayContext *ctx, Transforms *transforms) {
    // TODO
}

void renderQuad(const ChunkMesh *mesh, SMD_PRIM *primitive, DisplayContext *ctx, Transforms *transforms) {
    // TODO: Generalise for textured and non-textured
    int p;
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->vertices);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->normals);
    POLY_FT4 *pol4 = (POLY_FT4 *) ctx->primitive;
    gte_ldv3(
        &verticesIter[primitive->v0],
        &verticesIter[primitive->v1],
        &verticesIter[primitive->v2]
    );
    printf(
        "Setting vertices 1: {%d,%d,%d}\n",
        verticesIter[primitive->v0].vx,
        verticesIter[primitive->v0].vy,
        verticesIter[primitive->v0].vz
    );
    printf(
        "Setting vertices 2: {%d,%d,%d}\n",
        verticesIter[primitive->v1].vx,
        verticesIter[primitive->v1].vy,
        verticesIter[primitive->v1].vz
    );
    printf(
        "Setting vertices 2: {%d,%d,%d}\n",
        verticesIter[primitive->v2].vx,
        verticesIter[primitive->v2].vy,
        verticesIter[primitive->v2].vz
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    gte_nclip();
    gte_stopz(&p);
    printf("Set RTP\n");
    if (p < 0) {
        return;
    }
    // Average screen Z result for four primtives
    gte_avsz4();
    gte_stotz(&p);
    printf("Average screen Z\n");
    // (the shift right operator is to scale the depth precision)
    if (p >> 2 <= 0 || p >> 2 >= ORDERING_TABLE_LENGTH) {
        return;
    }
    // Initialize a textured quad primitive
    setPolyFT4(pol4);
    // Set the projected vertices to the primitive
    gte_stsxy0(&pol4->x0);
    gte_stsxy1(&pol4->x1);
    gte_stsxy2(&pol4->x2);
    printf("Set projected vertices\n");
    // Compute the last vertex and set the result
    gte_ldv0(&verticesIter[primitive->v3]);
    printf(
        "Setting vertices 3: {%d,%d,%d}\n",
        verticesIter[primitive->v3].vx,
        verticesIter[primitive->v3].vy,
        verticesIter[primitive->v3].vz
    );
    gte_rtps();
    gte_stsxy(&pol4->x3);
    printf("Set last vert\n");
    // Test if quad is off-screen, discard if so
    if (quad_clip(
        &ctx->screen_clip,
        (DVECTOR *) &pol4->x0,
        (DVECTOR *) &pol4->x1,
        (DVECTOR *) &pol4->x2,
        (DVECTOR *) &pol4->x3)) {
        return;
    }
    printf("Checked quad clip\n");
    // Load primitive color even though gte_ncs() doesn't use it.
    // This is so the GTE will output a color result with the
    // correct primitive code.
    if (primitive->code) {
        setRGB0(
            pol4,
            primitive->r0,
            primitive->g0,
            primitive->b0
        );
        printf("Set RGB tint\n");
    }
    gte_ldrgb(&pol4->r0);
    // Load the face normal
    gte_ldv0(&normalsIter[primitive->n0]);
    printf(
        "Loaded normal: {%d,%d,%d}\n",
        normalsIter[primitive->n0].vx,
        normalsIter[primitive->n0].vy,
        normalsIter[primitive->n0].vz
    );
    // Apply RGB tinting to lighting calculation result on the basis
    // that it is enabled. This corresponds to the column based calc
    if (primitive->code) {
        // Normal Color Column Single
        gte_nccs();
    } else {
        // Normal Color Single
        gte_ncs();
    }
    // Store result to the primitive
    gte_strgb(&pol4->r0);
    printf("Read r0\n");
    // Set texture coords and dimensions
    setUVWH(
        pol4,
        primitive->tu0,
        primitive->tv0,
        primitive->tu1,
        primitive->tv1
    );
    printf(
        "Set UVWH %d,%d,%d,%d\n",
        primitive->tu0,
        primitive->tv0,
        primitive->tu1,
        primitive->tv1
    );
    // Bind texture page and colour look-up-table
    pol4->tpage = primitive->tpage;
    pol4->clut = primitive->clut;
    // Sort primitive to the ordering table
    addPrim(ctx->db[ctx->active].ordering_table + (p >> 2), pol4);
    printf("Added primitive\n");
    // Advance to make another primitive
    ctx->primitive++;
}

void chunkMeshRender(const ChunkMesh *mesh, DisplayContext *ctx, Transforms *transforms) {
    printf("Primitives: %d\n", cvector_size(mesh->primitives));
    for (cvector_iterator(SMD_PRIM) primitive = cvector_begin(mesh->primitives);
         primitive != cvector_end(mesh->primitives); primitive++) {
        printf("Primitive type: %d @ %p\n", primitive->prim_id.type, primitive);
        switch (primitive->prim_id.type) {
            case PRIM_TYPE_LINE:
                renderLine(primitive, ctx, transforms);
                break;
            case PRIM_TYPE_TRI:
                renderTriangle(primitive, ctx, transforms);
                break;
            case PRIM_TYPE_QUAD:
                renderQuad(mesh, primitive, ctx, transforms);
                break;
            default:
                printf("Unknown primitive type: %d\n", primitive->prim_id.type);
                return;
        }
    }
}
