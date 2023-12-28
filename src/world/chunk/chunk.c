#include "chunk.h"

#include <clip.h>
#include <inline_c.h>
#include <stdbool.h>
#include <smd/smd.h>

void chunkInit(Chunk* chunk) {
    chunkMeshInit(&chunk->mesh);
    // chunkGenerate3DHeightMap(chunk, {});
    chunkGenerateMesh(chunk);
}

void chunkDestroy(const Chunk* chunk) {
    chunkMeshDestroy(&chunk->mesh);
}

void chunkClearMesh(Chunk* chunk) {
    ChunkMesh* mesh = &chunk->mesh;
    chunkMeshClear(mesh);
    SMD* smd = &mesh->smd;
    smd->id[0] = 'S';
    smd->id[1] = 'M';
    smd->id[2] = 'D';
    smd->version = 0x01;
    smd->flags = 0x0;
    smd->p_verts = cvector_begin(mesh->vertices);
    smd->p_norms = cvector_begin(mesh->normals);
    smd->p_prims = cvector_begin(mesh->primitives);
}

#define CHUNK_DIRECTIONS 3

BlockID worldGetBlock(int x, int y, int z);

typedef struct {
    int16_t block;
    int8_t normal;
} Mask;

#define compareMask(m1, m2) ((m1).block == (m2).block && (m1).normal == (m2).normal)

#define vec3Mul(type, vec, scalar) ((type) { (vec).vx * (scalar), (vec).vy * (scalar), (vec).vz * (scalar) })

void createQuad(Chunk* chunk,
                const Mask* mask,
                const VECTOR axisMask,
                const int width,
                const int height,
                const VECTOR v1,
                const VECTOR v2,
                const VECTOR v3,
                const VECTOR v4) {
    const SVECTOR normal = (SVECTOR){
        axisMask.vx * mask->normal,
        axisMask.vy * mask->normal,
        axisMask.vz * mask->normal
    };
    ChunkMesh* mesh = &chunk->mesh;
    SMD* smd = &mesh->smd;
    // Construct a new POLY_FT4 (textured quad) primtive for this face
    smd->n_prims += 1;
    SMD_PRIM primtive = {0};
    primtive.prim_id = (SMD_PRI_TYPE) {
        .type = PRIM_TYPE_QUAD,
        .l_type = PRIM_LIGHTING_FLAT,
        .c_type = PRIM_COLOURING_GOURAUD,
        .texture = 1,
        .blend = 0, // TODO: Check this
        .zoff = 0, // TODO: Check this
        .nocull = 0,
        .mask = 0,
        .texwin = 0,
        .texoff = 0,
        .reserved = 0,
        .len = 4 + 8 + 4 + 8 + 4 // Some wizardry based on PSn00bSDK/tools/smxlink/main.cpp lines 518-644
    };

    // Construct vertices relative to chunk mesh top left origin
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v1, BLOCK_SIZE));
    primtive.v0 = smd->n_verts;
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v2, BLOCK_SIZE));
    primtive.v1 = smd->n_verts + 1;
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v3, BLOCK_SIZE));
    primtive.v2 = smd->n_verts + 2;
    cvector_push_back(mesh->vertices, vec3Mul(SVECTOR, v4, BLOCK_SIZE));
    primtive.v3 = smd->n_verts + 3;

    // Create normals for each vertex
    // FIXME: Surely we can just have one normal for a face and then reference it 4 times,
    //        instead of needing 4 distinct normal instances to reference
    cvector_push_back(mesh->normals, normal);
    primtive.n0 = smd->n_norms;
    cvector_push_back(mesh->normals, normal);
    primtive.n1 = smd->n_norms + 1;
    cvector_push_back(mesh->normals, normal);
    primtive.n2 = smd->n_norms + 2;
    cvector_push_back(mesh->normals, normal);
    primtive.n3 = smd->n_norms + 3;

    Texture* texture = &textures[BLOCKS[mask->block].cube->texture];
    primtive.tpage = texture->tpage;
    primtive.clut = texture->clut;

    // TODO: Need to figure out how to get texture wrapping to work
    //       across a quad for a single 16x16 smapler of the given tpage + clut
    if (normal.vx == 1 || normal.vx == -1) {
        primtive.tu0 = width;
        primtive.tv0 = height;

        primtive.tu1 = 0;
        primtive.tv1 = height;

        primtive.tu2 = width;
        primtive.tv2 = 0;

        primtive.tu3 = 0;
        primtive.tu3 = 0;
    } else {
        primtive.tu0 = height;
        primtive.tv0 = width;

        primtive.tu1 = height;
        primtive.tv1 = 0;

        primtive.tu2 = 0;
        primtive.tv2 = width;

        primtive.tu3 = 0;
        primtive.tv3 = 0;
    }
    // These need to be at the end since we use them as base reference for indices
    smd->n_verts += 4;
    smd->n_norms += 4; // FIXME: This will need to be adjusted to +=1 if we can use just one normal instance per face
}

void chunkGenerateMesh(Chunk *chunk) {
    // TODO: Clear mesh field before generating new one
    for (int axis = 0; axis < CHUNK_DIRECTIONS; axis++) {
        int k, width, height;
        const int axis1 = (axis + 1) % CHUNK_DIRECTIONS;
        const int axis2 = (axis + 2) % CHUNK_DIRECTIONS;
        // FIXME: These can be VECTOR instances instead of arrays, but will come at the cost of an extra padding field
        //        is that worth it or should thee just be left as is?
        int deltaAxis1[CHUNK_DIRECTIONS] = {0};
        int deltaAxis2[CHUNK_DIRECTIONS] = {0};
        int chunkIter[CHUNK_DIRECTIONS] = {0};
        // TODO: Make this a uint8_t[CHUNK_SIZE] array where each bit is a bool, int type will need to change
        //       if CHUNK_SIZE is increased from 8 to 16 (uint8_t -> uint16_t)
        Mask mask[CHUNK_SIZE * CHUNK_SIZE];
        for (chunkIter[axis] = -1; chunkIter[axis] < CHUNK_SIZE;) {
            int axisMask[CHUNK_DIRECTIONS] = {0};
            axisMask[axis] = 1;
            // Compute mask
            uint8_t n = 0;
            for (chunkIter[axis2] = 0; chunkIter[axis1] < CHUNK_SIZE; chunkIter[axis2]++) {
                for (chunkIter[axis1] = 0; chunkIter[axis1] < CHUNK_SIZE; chunkIter[axis1]++) {
                    const BlockID currentBlock = chunkGetBlock(
                        chunk,
                        chunkIter[0] + chunk->position.vx,
                        chunkIter[1] + chunk->position.vy,
                        chunkIter[2] + chunk->position.vz
                    );
                    const bool currentOpaque = BLOCKS[currentBlock].type == SOLID;
                    const BlockID compareBlock = chunkGetBlock(
                        chunk,
                        chunkIter[0] + axisMask[0] + chunk->position.vx,
                        chunkIter[1] + axisMask[1] + chunk->position.vy,
                        chunkIter[2] + axisMask[2] + chunk->position.vz
                    );
                    const bool compareOpaque = BLOCKS[compareBlock].type == SOLID;
                    if (currentOpaque == compareOpaque) {
                        mask[n++] = (Mask){-1, 0};
                    } else if (currentOpaque) {
                        mask[n++] = (Mask){(int16_t) currentBlock, 1};
                    } else {
                        mask[n++] = (Mask){(int16_t) compareBlock, -1};
                    }
                }
            }
            chunkIter[axis]++;
            n = 0;
            // Generate a mesh from mask using lexicographic ordering
            // looping over each block in this slice of the chunk
            for (int j = 0; j < CHUNK_SIZE; j++) {
                for (int i = 0; i < CHUNK_SIZE;) {
                    if (mask[n].normal == 0) {
                        i++;
                        n++;
                        continue;
                    }
                    const Mask currentMask = mask[n];
                    chunkIter[axis1] = i;
                    chunkIter[axis2] = j;
                    // Compute the width of this quad and store it in w
                    // This is done by searching along the current axis until mask[n + w] is false
                    for (width = 1; i + width < CHUNK_SIZE && compareMask(mask[n + width], currentMask); width++) {}

                    // Compute the height of this quad and store it in h
                    // This is done by checking if every block next to this row (range 0 to w) is also part of the mask.
                    // For example, if w is 5 we currently have a quad of dimensions 1 x 5. To reduce triangle count,
                    // greedy meshing will attempt to expand this quad out to CHUNK_SIZE x 5, but will stop if it reaches a hole in the mask
                    bool done = false;
                    for (height = 1; j + height < CHUNK_SIZE; height++) {
                        // Check each block next to this quad
                        for (k = 0; k < width; ++k) {
                            // If there's a hole in the mask, exit
                            if (compareMask(mask[n + k + height * CHUNK_SIZE], currentMask)) continue;
                            done = true;
                            break;
                        }
                        if (done) break;
                    }
                    deltaAxis1[axis1] = width;
                    deltaAxis2[axis2] = height;
                    // FIXME: Avoid creating structs just to pass parameters to createQuad(...)
                    createQuad(
                        chunk,
                        &currentMask,
                        (VECTOR) {axisMask[0], axisMask[1], axisMask[2]},
                        width,
                        height,
                        // NOTE: These verticies need to be adjusted to fit block size
                        //       they are currently just indices into the chunk from top left.
                        (VECTOR) {chunkIter[0], chunkIter[1], chunkIter[2]},
                        (VECTOR){
                            chunkIter[0] + deltaAxis1[0],
                            chunkIter[1] + deltaAxis1[1],
                            chunkIter[2] + deltaAxis1[2]
                        },
                        (VECTOR){
                            chunkIter[0] + deltaAxis2[0],
                            chunkIter[1] + deltaAxis2[1],
                            chunkIter[2] + deltaAxis2[2]
                        },
                        (VECTOR){
                            chunkIter[0] + deltaAxis1[0] + deltaAxis2[0],
                            chunkIter[1] + deltaAxis1[1] + deltaAxis2[1],
                            chunkIter[2] + deltaAxis1[2] + deltaAxis2[2]
                        }
                    );
                    deltaAxis1[axis1] = 0;
                    deltaAxis2[axis2] = 0;
                    for (int l = 0; l < height; l++) {
                        for (k = 0; k < width; k++) {
                            mask[n + k + l * CHUNK_SIZE] = (Mask){0, 0};
                        }
                    }
                    i += width,
                    n += width;
                }
            }
        }
    }
}

void chunkRender(Chunk* chunk, DisplayContext* ctx, Transforms* transforms) {
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
        if (attributes.tint.use) {
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
        if (attributes.tint.use) {
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

#define checkIndexOOB(x, y, z) ((x) >= CHUNK_SIZE || (x) < 0 \
	|| (y) >= CHUNK_SIZE || (y) < 0 \
	|| (z) >= CHUNK_SIZE || (z) < 0)

BlockID chunkGetBlock(const Chunk *chunk, const int x, const int y, const int z) {
    if (checkIndexOOB(x, y, z)) return 0;
    return chunk->data[chunkBlockIndex(x, y, z)];
}

BlockID chunkGetBlockVec(const Chunk *chunk, const VECTOR *position) {
    if (checkIndexOOB(position->vx, position->vy, position->vz)) return 0;
    return chunk->data[chunkBlockIndex(position->vx, position->vy, position->vz)];
}
