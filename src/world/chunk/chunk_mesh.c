#include "chunk_mesh.h"

#include <clip.h>
#include <inline_c.h>
#include <stdlib.h>

#include "../../structure/cvector.h"
#include "../../structure/cvector_utils.h"
#include "../../blocks/block.h"

void __primtiveDestructor(void* elem) {
    memset(elem, 0, sizeof(SMD_PRIM));
}

void __svectorDestructor(void* elem) {
    memset(elem, 0, sizeof(SVECTOR));
}

void chunkMeshInit(ChunkMesh* mesh) {
    // NOTE: This null init is important for cvector to ensure allocation is done initially
    SMD_PRIM* p_prims = NULL;
    mesh->p_verts = NULL;
    mesh->p_norms = NULL;
    cvector_init(p_prims, MESH_PRIMITIVE_VEC_INITIAL_CAPCITY, __primtiveDestructor);
    mesh->p_prims = p_prims;
    cvector_init(mesh->p_verts, MESH_VERTEX_VEC_INITIAL_CAPCITY, __svectorDestructor);
    cvector_init(mesh->p_norms, MESH_NORMAL_VEC_INITIAL_CAPCITY, __svectorDestructor);
}

void chunkMeshDestroy(const ChunkMesh* mesh) {
    cvector_free((SMD_PRIM*) mesh->p_prims);
    cvector_free(mesh->p_verts);
    cvector_free(mesh->p_norms);
}

void chunkMeshClear(ChunkMesh* mesh) {
    cvector_clear((SMD_PRIM*) mesh->p_prims);
    cvector_clear(mesh->p_verts);
    cvector_clear(mesh->p_norms);
    mesh->id[0] = 'S';
    mesh->id[1] = 'M';
    mesh->id[2] = 'D';
    mesh->version = 0x01;
    mesh->flags = 0x0;
    mesh->n_verts = 0;
    mesh->n_norms = 0;
    mesh->n_prims = 0;
}

// TODO: Move these to SMD renderer file as general methods
void renderLine(SMD_PRIM* primitive, RenderContext* ctx, Transforms* transforms) {
    // TODO
}

void renderTriangle(SMD_PRIM* primitive, RenderContext* ctx, Transforms* transforms) {
    // TODO
}

void renderQuad(const ChunkMesh* mesh, SMD_PRIM* primitive, RenderContext* ctx, Transforms* transforms) {
    // TODO: Generalise for textured and non-textured
    int p;
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->p_verts);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->p_norms);
    const RECT tex_window = (RECT){
        primitive->tu0 >> 3,
        primitive->tv0 >> 3,
        BLOCK_TEXTURE_SIZE >> 3,
        BLOCK_TEXTURE_SIZE >> 3
    };
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
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
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
    }
    // Average screen Z result for four primtives
    gte_avsz4();
    gte_stotz(&p);
    // (the shift right operator is to scale the depth precision)
    if (p >> 2 <= 1 || p >> 2 >= ORDERING_TABLE_LENGTH) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
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
        (DVECTOR*) &pol4->x0,
        (DVECTOR*) &pol4->x1,
        (DVECTOR*) &pol4->x2,
        (DVECTOR*) &pol4->x3)) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
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
    uint32_t* ot_object = allocateOrderingTable(ctx, p >> 2);
    addPrim(ot_object, pol4);
    // Advance to make another primitive
    // Bind a texture window to ensure wrapping across merged block face primitives
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    ot_object = allocateOrderingTable(ctx, p >> 2);
    addPrim(ot_object, ptwin);
}

void chunkMeshRender(const ChunkMesh* mesh, RenderContext* ctx, Transforms* transforms) {
    const SMD_PRIM* p_prims = (SMD_PRIM*) mesh->p_prims;
    cvector_iterator(SMD_PRIM) primitive;
    cvector_for_each_in(primitive, p_prims) {
        switch (primitive->prim_id.type) {
            case SMD_PRI_TYPE_LINE:
                renderLine(primitive, ctx, transforms);
                break;
            case SMD_PRI_TYPE_TRIANGLE:
                renderTriangle(primitive, ctx, transforms);
                break;
            case SMD_PRI_TYPE_QUAD:
                renderQuad(mesh, primitive, ctx, transforms);
                break;
            default:
                printf(
                    "[ERROR] ChunkMesh - Unknown primitive type: %d\n",
                    primitive->prim_id.type
                );
                return;
        }
    }
}
