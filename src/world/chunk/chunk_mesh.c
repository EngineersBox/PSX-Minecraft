#include "chunk_mesh.h"

#include <clip.h>
#include <inline_c.h>
#include <stdlib.h>

#include "../../blocks/block.h"

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
    // !BUG: These small pre-allocated sizes cause issues with realloc (out of bounds read/write)
    printf("Init primitives\n");
    cvector_init(mesh->primitives, MESH_PRIMITIVE_VEC_INITIAL_CAPCITY, __primtiveDestructor);
    printf("Init vertices\n");
    cvector_init(mesh->vertices, MESH_VERTEX_VEC_INITIAL_CAPCITY, __svectorDestructor);
    printf("Init normals\n");
    cvector_init(mesh->normals, MESH_NORMAL_VEC_INITIAL_CAPCITY, __svectorDestructor);
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

// TODO: Move these to SMD renderer file as general methods
int renderLine(SMD_PRIM *primitive, DisplayContext *ctx, Transforms *transforms) {
    // TODO
    return 0;
}

int renderTriangle(SMD_PRIM *primitive, DisplayContext *ctx, Transforms *transforms) {
    // TODO
    return 0;
}

int renderQuad(const ChunkMesh *mesh, SMD_PRIM *primitive, DisplayContext *ctx, Transforms *transforms) {
    // TODO: Generalise for textured and non-textured
    int p;
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->vertices);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->normals);
    const RECT tex_window = (RECT){
        primitive->tu0 >> 3,
        primitive->tv0 >> 3,
        BLOCK_TEXTURE_SIZE >> 3,
        BLOCK_TEXTURE_SIZE >> 3
    };
    POLY_FT4 *pol4 = (POLY_FT4*) ctx->primitive;
    gte_ldv3(
        &verticesIter[primitive->v0],
        &verticesIter[primitive->v1],
        &verticesIter[primitive->v2]
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    gte_nclip();
    gte_stopz(&p);
    // Avoid negative depth (behind camera) and zero
    // for constraint clearing primitive in OT
    if (p <= 0) {
        return -1;
    }
    // Average screen Z result for four primtives
    gte_avsz4();
    gte_stotz(&p);
    // (the shift right operator is to scale the depth precision)
    if (p >> 2 <= 0 || p >> 2 >= ORDERING_TABLE_LENGTH) {
        return -1;
    }
    // Initialize a textured quad primitive
    setPolyFT4(pol4);
    // Set the projected vertices to the primitive
    gte_stsxy0(&pol4->x0);
    gte_stsxy1(&pol4->x1);
    gte_stsxy2(&pol4->x2);
    // Compute the last vertex and set the result
    gte_ldv0(&verticesIter[primitive->v3]);
    gte_rtps();
    gte_stsxy(&pol4->x3);
    // Test if quad is off-screen, discard if so
    if (quad_clip(
        &ctx->screen_clip,
        (DVECTOR *) &pol4->x0,
        (DVECTOR *) &pol4->x1,
        (DVECTOR *) &pol4->x2,
        (DVECTOR *) &pol4->x3)) {
        return -1;
    }
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
    }
    gte_ldrgb(&pol4->r0);
    // Load the face normal
    gte_ldv0(&normalsIter[primitive->n0]);
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
    // Set texture coords and dimensions
    setUVWH(
        pol4,
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
    // Advance to make another primitive
    pol4++;
    ctx->primitive = (char*) pol4;
    // Bind a texture window to ensure wrapping across merged block face primitives
    DR_TWIN* ptwin = (DR_TWIN*) ctx->primitive;
    setTexWindow(ptwin, &tex_window);
    addPrim(ctx->db[ctx->active].ordering_table + (p >> 2), ptwin);
    ptwin++;
    ctx->primitive = (char*) ptwin;
    return p >> 2;
}

#define updateOffset(new_offset) ({\
    if ((new_offset) > 0 && (new_offset) < ordering_table_offset) {\
        ordering_table_offset = (new_offset);\
    }\
})

void chunkMeshRender(const ChunkMesh *mesh, DisplayContext *ctx, Transforms *transforms) {
    // printf("Primitives: %d\n", cvector_size(mesh->primitives));
    int ordering_table_offset = ORDERING_TABLE_LENGTH - 1;
    for (cvector_iterator(SMD_PRIM) primitive = cvector_begin(mesh->primitives);
         primitive != cvector_end(mesh->primitives); primitive++) {
        // printf("[%d] Primitive type: %d @ %p\n", i++, primitive->prim_id.type, primitive);
        switch (primitive->prim_id.type) {
            case PRIM_TYPE_LINE:
                updateOffset(renderLine(primitive, ctx, transforms));
                break;
            case PRIM_TYPE_TRI:
                updateOffset(renderTriangle(primitive, ctx, transforms));
                break;
            case PRIM_TYPE_QUAD:
                updateOffset(renderQuad(mesh, primitive, ctx, transforms));
                break;
            default:
                printf("[ERROR] ChunkMesh - Unknown primitive type: %d\n", primitive->prim_id.type);
                return;
        }
    }
    // if (ordering_table_offset == ORDERING_TABLE_LENGTH - 1) {
    //     return;
    // }
    // // Clear window constraints
    // DR_TWIN* ptwin = (DR_TWIN*) ctx->primitive;
    // const RECT tex_window = {
    //     .x = 0,
    //     .y = 0,
    //     .w = 0,
    //     .h = 0
    // };
    // setTexWindow(ptwin, &tex_window);
    // addPrim(ctx->db[ctx->active].ordering_table + ordering_table_offset - 1, ptwin);
    // ptwin++;
    // ctx->primitive = (char*) ptwin;
}
