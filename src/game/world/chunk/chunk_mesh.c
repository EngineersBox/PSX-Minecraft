#include "chunk_mesh.h"

#include <inline_c.h>
#include <stdlib.h>
#include <psxgpu.h>

#include "../../structure/cvector.h"
#include "../../structure/cvector_utils.h"
#include "../../blocks/block.h"
#include "../../structure/primitive/clip.h"
#include "../../logging/logging.h"
#include "../../render/commands.h"
#include "chunk_structure.h"

FWD_DECL u8 lightMapGetValue(const LightMap lightmap, const VECTOR position);
FWD_DECL u8 lightLevelApplicable(const u8 light_value);

#ifndef QUAD_DUAL_TRI_NCLIP
#define QUAD_DUAL_TRI_NCLIP 0
#endif

void __primtiveDestructor(void* elem) {
    memset(elem, 0, sizeof(MeshPrimitive));
}

void __svectorDestructor(void* elem) {
    memset(elem, 0, sizeof(SVECTOR));
}

void chunkMeshInit(ChunkMesh* mesh) {
    #pragma GCC unroll 6
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        // NOTE: This null init is important for cvector to ensure allocation is done initially
        Mesh* face_mesh = &mesh->face_meshes[i];
        cvector(MeshPrimitive) p_prims = NULL;
        cvector_init(p_prims, 0, NULL);
        face_mesh->p_prims = p_prims;
        face_mesh->p_verts = NULL;
        cvector_init(face_mesh->p_verts, 0, NULL);
        face_mesh->p_norms = NULL;
        cvector_init(face_mesh->p_norms, 0, NULL);
    }
}

void chunkMeshDestroy(const ChunkMesh* mesh) {
    #pragma GCC unroll 6
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        const Mesh* face_mesh = &mesh->face_meshes[i];
        cvector_free((MeshPrimitive*) face_mesh->p_prims);
        cvector_free(face_mesh->p_verts);
        cvector_free(face_mesh->p_norms);
    }
}

void chunkMeshClear(ChunkMesh* mesh) {
    #pragma GCC unroll 6
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        Mesh* face_mesh = &mesh->face_meshes[i];
        cvector_clear((cvector(MeshPrimitive)) face_mesh->p_prims);
        cvector_clear(face_mesh->p_verts);
        cvector_clear(face_mesh->p_norms);
        face_mesh->n_verts = 0;
        face_mesh->n_norms = 0;
        face_mesh->n_prims = 0;
    }
}

static void renderLine(MeshPrimitive* primitive, RenderContext* ctx, Transforms* transforms) {
    // TODO
}

static void renderTriangle(MeshPrimitive* primitive, RenderContext* ctx, Transforms* transforms) {
    // TODO
}

const RECT lightmap_merge_offscreen = (RECT) {
    .x = 832,
    .y = 0,
    .w = (CHUNK_SIZE * BLOCK_TEXTURE_SIZE),
    .h = ((CHUNK_SIZE + 1) * BLOCK_TEXTURE_SIZE) // Additional here is to account for breaking texture at top
};

static void renderQuad(const Mesh* mesh,
                       const MeshPrimitive* primitive,
                       RenderContext* ctx,
                       Transforms* transforms) {
    // TODO: Generalise for textured and non-textured
    int p;
    // int dp;
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->p_verts);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->p_norms);
    const RECT tex_window = (RECT){
        // All in units of 8 pixels, hence right shift by 3
        .w = BLOCK_TEXTURE_SIZE >> 3,
        .h = BLOCK_TEXTURE_SIZE >> 3,
        .x = primitive->tu0 >> 3,
        .y = primitive->tv0 >> 3
    };
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
#if QUAD_DUAL_TRI_NCLIP
    // Initialize a textured quad primitive
    setPolyFT4(pol4);
    gte_ldv3(
        &verticesIter[primitive->v3],
        &verticesIter[primitive->v2],
        &verticesIter[primitive->v1]
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
    gte_stsxy0(&pol4->x3);
#endif
    gte_ldv3(
        &verticesIter[primitive->v0],
        &verticesIter[primitive->v1],
        &verticesIter[primitive->v2]
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    gte_nclip();
    gte_stopz(&p);
    // gte_stdp(&dp);
    // Avoid negative depth (behind camera) and zero
    // for constraint clearing primitive in OT
    if (p <= 0) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
    }
    // Average screen Z result for three vertices
    gte_avsz3();
    gte_stotz(&p);
    if (p <= 0 || p >= ORDERING_TABLE_LENGTH) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
    }
#if !QUAD_DUAL_TRI_NCLIP
    // Initialize a textured quad primitive
    setPolyFT4(pol4);
#endif
    // Set the projected vertices to the primitive
    gte_stsxy0(&pol4->x0);
    gte_stsxy1(&pol4->x1);
    gte_stsxy2(&pol4->x2);
#if !QUAD_DUAL_TRI_NCLIP
    // Compute the last vertex and set the result
    gte_ldv0(&verticesIter[primitive->v3]);
    gte_rtps();
    gte_stsxy(&pol4->x3);
#endif
    // Test if quad is off-screen, discard if so
    if (quadClip(
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
    if (primitive->tint) {
        setRGB0(
            pol4,
            primitive->r,
            primitive->g,
            primitive->b
        );
    }
    gte_ldrgb(&pol4->r0);
    // Load the face normal
    gte_ldv0(&normalsIter[primitive->n0]);
    // gte_lddp(&dp);
    // Apply RGB tinting to lighting calculation result on the basis
    // that it is enabled.
    if (primitive->tint) {
        // Normal Color Column Single
        gte_nccs();
    } else {
        // Normal Color Single
        gte_ncs();
    }
    // Store result to the primitive
    gte_strgb(&pol4->r0);
    // Set texture coords and dimensions
    // TODO: Blit texture to off-screen tpage and then apply lighting
    //       changes for each 16x16 section of it. Finally reference
    //       the off-screen tpage here as the target to draw to the
    //       in-world poly. This is essentially procedural texturing,
    //       the same as how the breaking overlay is done.
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
    u32* ot_object = allocateOrderingTable(ctx, p);
    addPrim(ot_object, pol4);
    // Bind a texture window to ensure wrapping across merged block face primitives
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    ot_object = allocateOrderingTable(ctx, p);
    addPrim(ot_object, ptwin);
}

void chunkMeshRenderFaceDirection(const Mesh* mesh, RenderContext* ctx, Transforms* transforms) {
    MeshPrimitive* p_prims = (MeshPrimitive*) mesh->p_prims;
    cvector_iterator(MeshPrimitive) primitive;
    cvector_for_each_in(primitive, p_prims) {
        switch (primitive->type) {
            case MESH_PRIM_TYPE_LINE:
                renderLine(primitive, ctx, transforms);
                break;
            case MESH_PRIM_TYPE_TRIANGLE:
                renderTriangle(primitive, ctx, transforms);
                break;
            case MESH_PRIM_TYPE_QUAD:
                renderQuad(mesh, primitive, ctx, transforms);
                break;
            default:
                printf(
                    "[ERROR] ChunkMesh - Unknown primitive type: %d\n",
                    primitive->type
                );
            return;
        }
    }
}

UNUSED bool faceDirectionHidden(RenderContext* ctx, FaceDirection face_dir) {
    // TODO: Implement face direction culling, determing if faces in this
    //       direction are visible to the camera
    return false;
}

void chunkMeshRender(const ChunkMesh* mesh, RenderContext* ctx, Transforms* transforms) {
    // bool skip_check[6] = {false};
    #pragma GCC unroll 6
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        // if (skip_check[i] || faceDirectionHidden(ctx, i)) {
        //     continue;
        // }
        // // If we have determined that a face is visible, then the
        // // opposite face direction is necesserily not visible
        // switch (i) {
        //     case FACE_DIR_DOWN: skip_check[1] = true; break;
        //     case FACE_DIR_UP: skip_check[0] = true; break;
        //     case FACE_DIR_LEFT: skip_check[3] = true; break;
        //     case FACE_DIR_RIGHT: skip_check[2] = true; break;
        //     case FACE_DIR_BACK: skip_check[5] = true; break;
        //     case FACE_DIR_FRONT: skip_check[4] = true; break;
        // }
        chunkMeshRenderFaceDirection(
            &mesh->face_meshes[i],
            ctx,
            transforms
        );
    }
}
