#include "chunk_mesh.h"

#include <inline_c.h>
#include <stdlib.h>

#include "../../structure/cvector.h"
#include "../../structure/cvector_utils.h"
#include "../../blocks/block.h"
#include "../../structure/primitive/clip.h"
#include "../../logging/logging.h"
#include "../../render/commands.h"
#include "chunk_structure.h"
#include "psxgpu.h"

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

// TODO: Move these to SMD renderer file as general methods
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

static void renderQuad(const Mesh* mesh, MeshPrimitive* primitive, RenderContext* ctx, Transforms* transforms) {
    // TODO: Generalise for textured and non-textured
    int p;
    // int dp;
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->p_verts);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->p_norms);
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
    /*gte_strgb(&pol4->r1);*/
    /*gte_strgb(&pol4->r2);*/
    /*gte_strgb(&pol4->r3);*/
    // Set texture coords and dimensions
    setUVWH(
        pol4,
        0,
        BLOCK_TEXTURE_SIZE,
        primitive->tu1,
        primitive->tv1
    );
    // Testing colour modulation with Gouraud shading
    /*setRGB1(pol4, 128, 0, 0);*/
    /*setRGB2(pol4, 0, 128, 0);*/
    /*setRGB3(pol4, 0, 0, 128);;*/
    // Bind off-screen merge texture page and colour look-up-table
    pol4->tpage = getTPage(
        2,
        1,
        lightmap_merge_offscreen.x,
        lightmap_merge_offscreen.y
    );
    pol4->clut = primitive->clut;
    // Sort primitive to the ordering table
    const u32* ot_entry = allocateOrderingTable(ctx, p);
    addPrim(ot_entry, pol4);
    // Reset texture window that will be enabled for applying lighting overlays
    RECT tex_window = (RECT) {0, 0, 0, 0};
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_entry, ptwin);
    /* ============================================= */
    /* ===  OFF-SCREEN PROCEDURAL LIGHT OVERLAY  === */
    /* ============================================= */
    // Sort drawing primitives to reset drawing area to default (OT is in reverse order so that's
    // why this is here and not after the loop).
    DR_AREA* area = (DR_AREA*) allocatePrimitive(ctx, sizeof(DR_AREA));
    const DB* inactive = &ctx->db[1 - ctx->active];
    setDrawArea(area, &inactive->draw_env.clip);
    addPrim(ot_entry, area);
    DR_OFFSET* offset = (DR_OFFSET*) allocatePrimitive(ctx, sizeof(DR_OFFSET));
    setDrawOffset(
        offset,
        inactive->draw_env.clip.x,
        inactive->draw_env.clip.y
    );
    addPrim(ot_entry, offset);
    const u32 prim_width = primitive->tu1;
    const u32 prim_height = primitive->tv1;
    /*for (u32 x = 0; x < prim_width; x += BLOCK_TEXTURE_SIZE) {*/
    /*    for (u32 y = 0; y < prim_height; y += BLOCK_TEXTURE_SIZE) {*/
    /*        TILE_16* tile = (TILE_16*) allocatePrimitive(ctx, sizeof(TILE_16));*/
    /*        setTile16(tile);*/
    /*        setXY0(tile, x, y + BLOCK_TEXTURE_SIZE);*/
    /*        // TODO: Query lightmap and set overlay colour based on light level*/
    /*        setRGB0(*/
    /*            tile,*/
    /*            (((x + y) << 4) % 3) * 0x80,*/
    /*            ((((x + y) << 4) + 1) % 3) * 0x80,*/
    /*            ((((x + y) << 4) + 2) % 3) * 0x80*/
    /*        );*/
    /*        setTransparency(tile, true);*/
    /*        addPrim(ot_entry, tile);*/
    /*    }*/
    /*}*/
    // Bit quad texture to off-screen location
    pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setSemiTrans(pol4, 0);
    setXYWH(
        pol4,
        0,
        BLOCK_TEXTURE_SIZE,
        prim_width,
        prim_height
    );
    if (primitive->tint) {
        setRGB0(
            pol4,
            primitive->r,
            primitive->g,
            primitive->b
        );
    } else {
        setRGB0(
            pol4,
            0x80,
            0x80,
            0x80
        );
    }
    setUVWH(
        pol4,
        primitive->tu0,
        primitive->tv0,
        primitive->tu1,
        primitive->tv1
    );
    pol4->tpage = primitive->tpage;
    pol4->clut = primitive->clut;
    addPrim(ot_entry, pol4);
    // Bind a texture window to ensure wrapping across merged block face primitives
    ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    tex_window = (RECT){
        // All in units of 8 pixels, hence right shift by 3
        .w = BLOCK_TEXTURE_SIZE >> 3,
        .h = BLOCK_TEXTURE_SIZE >> 3,
        .x = primitive->tu0 >> 3,
        .y = primitive->tv0 >> 3
    };
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_entry, ptwin);
    // Sort drawing bindings to ensure we target offscreen TPage area
    area = (DR_AREA*) allocatePrimitive(ctx, sizeof(DR_AREA));
    setDrawArea(area, &lightmap_merge_offscreen);
    addPrim(ot_entry, area);
    offset = (DR_OFFSET*) allocatePrimitive(ctx, sizeof(DR_OFFSET));
    setDrawOffset(
        offset,
        lightmap_merge_offscreen.x,
        lightmap_merge_offscreen.y
    );
    addPrim(ot_entry, offset);
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
