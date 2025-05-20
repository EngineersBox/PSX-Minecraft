#include "chunk_mesh.h"

#include <inline_c.h>
#include "../../../core/std/stdlib.h"
#include <psxgpu.h>

#include "../../../util/preprocessor.h"
#include "../../../structure/cvector.h"
#include "../../../structure/cvector_utils.h"
#include "../../../structure/primitive/clip.h"
#include "../../../logging/logging.h"
#include "../../../render/commands.h"
#include "../../blocks/block.h"
#include "chunk_structure.h"

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
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        Mesh face_mesh = {0};
        // NOTE: This null init is important for cvector to ensure allocation is done initially
        cvector(MeshPrimitive) p_prims = NULL;
        cvector_init(p_prims, 0, NULL);
        face_mesh.p_prims = p_prims;
        face_mesh.p_verts = NULL;
        cvector_init(face_mesh.p_verts, 0, NULL);
        face_mesh.p_norms = NULL;
        cvector_init(face_mesh.p_norms, 0, NULL);
        mesh->face_meshes[i] = face_mesh;
    }
}

void chunkMeshDestroy(const ChunkMesh* mesh) {
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        const Mesh* face_mesh = &mesh->face_meshes[i];
        cvector_free((MeshPrimitive*) face_mesh->p_prims);
        cvector_free(face_mesh->p_verts);
        cvector_free(face_mesh->p_norms);
    }
}

void chunkMeshClear(ChunkMesh* mesh) {
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
                       const LightLevel internal_light_level,
                       const MeshPrimitive* primitive,
                       bool subdivide,
                       RenderContext* ctx,
                       Transforms* transforms) {
    // TODO: Generalise for textured and non-textured
    int p;
    // int dp;
    cvector_iterator(SVECTOR) verticesIter = cvector_begin(mesh->p_verts);
    cvector_iterator(SVECTOR) normalsIter = cvector_begin(mesh->p_norms);
    const TextureWindow tex_window = textureWindowCreate(16, 16, primitive->tu0, primitive->tv0);
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
    // gte_stdp(&dp);
    // Avoid negative depth (behind camera) and zero
    // for constraint clearing primitive in OT
    if (p < 0) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
    }
    // Average screen Z result for three vertices
    gte_avsz3();
    gte_stotz(&p);
    if (p <= 0|| p >= ORDERING_TABLE_LENGTH) {
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
    if (quadClip(
        &ctx->screen_clip,
        (DVECTOR*) &pol4->x0,
        (DVECTOR*) &pol4->x1,
        (DVECTOR*) &pol4->x2,
        (DVECTOR*) &pol4->x3)) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return;
    }
    // NOTE: A downside of not re-meshing with updated sky light
    //       levels when the time-of-day changes is that the chance
    //       to merge with local block light values or dark regions
    //       is lost. However, these chances would be very unlikely
    //       to occur since block light or low light areas are generally
    //       not uniform (degrades in level each block further away
    //       from source) or are more likely to not be in sunlight
    //       anyway (save for cave entraces or around builds for
    //      instance) are are more lightly to be affected by chunk
    //       boundaries on light maps anyway. So the fact we do the
    //       adjustment of sunlight at each rendered frame is fine
    //       and we really don't gain much from the cost of remeshing
    //       as an alternative.
    const u16 light_level_colour_scalar = lightLevelColourScalar(
        internal_light_level,
        primitive->light_level
    );
    setRGB0(
        pol4,
        fixedMul(primitive->r, light_level_colour_scalar),
        fixedMul(primitive->g, light_level_colour_scalar),
        fixedMul(primitive->b, light_level_colour_scalar)
    );
    // Load primitive color even though gte_ncs() doesn't use it.
    // This is so the GTE will output a color result with the
    // correct primitive code.
    gte_ldrgb(&pol4->r0);
    // Load the face normal
    gte_ldv0(&normalsIter[primitive->n0]);
    // Apply RGB tinting to lighting calculation result on the basis
    // that it is enabled.
    // Normal Color Column Single
    gte_nccs();
    // Store result to the primitive
    gte_strgb(&pol4->r0);
    gte_avsz4();
    gte_stotz(&p);
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
    u32* ot_object = allocateOrderingTable(ctx, p);
    addPrim(ot_object, pol4);
    // Bind a texture window to ensure wrapping across merged block face primitives
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    ot_object = allocateOrderingTable(ctx, p);
    addPrim(ot_object, ptwin);
}

void chunkMeshRenderFaceDirection(const Mesh* mesh,
                                  const LightLevel internal_light_level,
                                  bool subdivide,
                                  RenderContext* ctx,
                                  Transforms* transforms) {
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
                renderQuad(
                    mesh,
                    internal_light_level,
                    primitive,
                    subdivide,
                    ctx,
                    transforms);
                break;
            default:
                errorAbort(
                    "[ERROR] ChunkMesh - Unknown primitive type: %d\n",
                    primitive->type
                );
            return;
        }
    }
}

#define POS_BETWEEN 0
#define POS_INCREASED 1
#define POS_DECREASED 2

static bool faceDirectionHidden(FaceDirection face_dir,
                                const VECTOR* aabb_closest_vertex,
                                u8 pos_state_x,
                                u8 pos_state_y,
                                u8 pos_state_z,
                                RenderContext* ctx) {
    /*const SVECTOR* normal = &FACE_DIRECTION_NORMALS[face_dir];*/
    switch (face_dir) {
        case FACE_DIR_DOWN:
            return pos_state_y == POS_DECREASED;
        case FACE_DIR_UP:
            return pos_state_y == POS_INCREASED;
        case FACE_DIR_LEFT:
            return pos_state_x == POS_INCREASED;
        case FACE_DIR_RIGHT:
            return pos_state_x == POS_DECREASED;
        case FACE_DIR_BACK:
            return pos_state_z == POS_INCREASED;
        case FACE_DIR_FRONT:
            return pos_state_z == POS_DECREASED;
    }
    return false;
}

void chunkMeshRender(const ChunkMesh* mesh,
                     const LightLevel internal_light_level,
                     const AABB* chunk_aabb,
                     bool subdivide,
                     RenderContext* ctx,
                     Transforms* transforms) {
    const VECTOR position = vec3_const_div(ctx->camera->position, ONE);
    #define posState(axis) \
        position.v##axis < chunk_aabb->min.v##axis \
            ? POS_DECREASED \
            : (position.v##axis >= chunk_aabb->max.v##axis \
                ? POS_INCREASED \
                : POS_BETWEEN \
            )
    const u8 pos_state_x = posState(x);
    const u8 pos_state_y = posState(y);
    const u8 pos_state_z = posState(z);
    #undef posState
    const VECTOR aabb_closest_vertex = aabbVertexClosestToPoint(
        chunk_aabb,
        &position
    );
    bool skip_check[6] = {false, false, false, false, false, false};
    for (int i = 0; i < FACE_DIRECTION_COUNT; i++) {
        if (skip_check[i] || faceDirectionHidden(
            i,
            &aabb_closest_vertex,
            pos_state_x,
            pos_state_y,
            pos_state_z,
            ctx
        )) {
            continue;
        }
        // If we have determined that a face is visible, then the
        // opposite face direction is necesserily not visible
        switch (i) {
            case FACE_DIR_DOWN: skip_check[1] = (bool) pos_state_y; break;
            case FACE_DIR_UP: skip_check[0] = (bool) pos_state_y; break;
            case FACE_DIR_LEFT: skip_check[3] = (bool) pos_state_x; break;
            case FACE_DIR_RIGHT: skip_check[2] = (bool) pos_state_x; break;
            case FACE_DIR_BACK: skip_check[5] = (bool) pos_state_z; break;
            case FACE_DIR_FRONT: skip_check[4] = (bool) pos_state_z; break;
        }
        chunkMeshRenderFaceDirection(
            &mesh->face_meshes[i],
            internal_light_level,
            subdivide,
            ctx,
            transforms
        );
    }
}
