#include "weather.h"

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "../resources/assets.h"
#include "../resources/asset_indices.h"
#include "../util/preprocessor.h"
#include "../game/world/position.h"
#include "../structure/primitive/clip.h"
#include "../structure/primitive/direction.h"
#include "../math/math_utils.h"
#include "../logging/logging.h"

FWD_DECL ChunkHeightmap* worldGetChunkHeightmap(World* world, const VECTOR* position);

POLY_FT4* createQuad(const SVECTOR vertices[4],
                     const SVECTOR normal,
                     RenderContext* ctx,
                     int* p) {
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    gte_ldv3(
        &vertices[0],
        &vertices[1],
        &vertices[2]
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    gte_nclip();
    gte_stopz(p);
    // gte_stdp(&dp);
    // Avoid negative depth (behind camera) and zero
    // for constraint clearing primitive in OT
    if (*p <= 0) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return NULL;
    }
    // Average screen Z result for three vertices
    gte_avsz3();
    gte_stotz(p);
    if (*p <= 0 || *p >= ORDERING_TABLE_LENGTH) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return NULL;
    }
    // Initialize a textured quad primitive
    setPolyFT4(pol4);
    // Set the projected vertices to the primitive
    gte_stsxy0(&pol4->x0);
    gte_stsxy1(&pol4->x1);
    gte_stsxy2(&pol4->x2);
    // Compute the last vertex and set the result
    gte_ldv0(&vertices[3]);
    gte_rtps();
    gte_stsxy(&pol4->x3);
    // Test if quad is off-screen, discard if so
    /*if (quadClip(*/
    /*    &ctx->screen_clip,*/
    /*    (DVECTOR*) &pol4->x0,*/
    /*    (DVECTOR*) &pol4->x1,*/
    /*    (DVECTOR*) &pol4->x2,*/
    /*    (DVECTOR*) &pol4->x3)) {*/
    /*    freePrimitive(ctx, sizeof(POLY_FT4));*/
    /*    return NULL;*/
    /*}*/
    setRGB0(pol4, 0xFF, 0xFF, 0xFF);
    // Load primitive color even though gte_ncs() doesn't use it.
    // This is so the GTE will output a color result with the
    // correct primitive code.
    gte_ldrgb(&pol4->r0);
    // Load the face normal
    gte_ldv0(&normal);
    // Apply RGB tinting to lighting calculation result on the basis
    // that it is enabled.
    // Normal Color Column Single
    gte_nccs();
    // Store result to the primitive
    gte_strgb(&pol4->r0);
    return pol4;
}

void weatherRender(const World* world,
                   const Player* player,
                   RenderContext* ctx,
                   Transforms* transforms) {
    static u16 render_ticks = 0;
    const VECTOR player_pos = vec3_const_div(
        player->physics_object.position,
        ONE_BLOCK
    );
    ChunkBlockPosition cb_pos;
    const Texture* texture = &textures[ASSET_TEXTURES_WEATHER_INDEX];
    int p = 0;
    u32 offset = 0;
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&VEC3_I16_ZERO, &omtx);
    TransMatrix(&omtx, &VEC3_I32_ZERO);
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
    for (i32 x = player_pos.vx - WEATHER_RENDER_RADIUS; x <= player_pos.vx + WEATHER_RENDER_RADIUS; x++) {
        for (i32 z = player_pos.vz - WEATHER_RENDER_RADIUS; z <= player_pos.vz + WEATHER_RENDER_RADIUS; z++) {
            /*if (offset++ % 2 == 0) {*/
            /*    continue;*/
            /*}*/
            const VECTOR pos = vec3_add(player_pos, vec3_i32(x, 0, z));
            cb_pos = worldToChunkBlockPosition(&pos, CHUNK_SIZE);
            ChunkHeightmap* heightmap = worldGetChunkHeightmap((World*) world, &cb_pos.chunk);
            const i32 top_solid_block_y = (*heightmap)[chunkHeightmapIndex(
                cb_pos.block.vx,
                cb_pos.block.vz
            )];
            const i32 y_bottom = max(player_pos.vy - WEATHER_RENDER_RADIUS, top_solid_block_y);
            const i32 y_top = max(player_pos.vy + WEATHER_RENDER_RADIUS, top_solid_block_y);
            if (y_bottom == y_top) {
                continue;
            }
            const bool current_is_snow = false; // TODO: Set this to true if we are in a tiaga biome
            // UV offsets
            srand(x * x * 3121 + x * 45238971 + z * z * 418711 + z * 13761);
            const u32 u_rand_offset = positiveModulo(rand(), WEATHER_TEXTURE_WIDTH);
            const u32 v_rand_offset = (positiveModulo(rand(), WEATHER_TEXTURE_HEIGHT) + render_ticks) % WEATHER_TEXTURE_HEIGHT;
            /*const u32 v_rand_offset = ((render_ticks >> 2) + (y_bottom * BLOCK_TEXTURE_SIZE)) % WEATHER_TEXTURE_HEIGHT;*/
            SVECTOR vertices[4];
            // X-axis
            vertices[0] = vec3_i16(
                x * BLOCK_SIZE,
                -y_top * BLOCK_SIZE,
                (z * BLOCK_SIZE) - (BLOCK_SIZE >> 1)
            );
            vertices[1] = vec3_i16(
                (x + 1) * BLOCK_SIZE,
                -y_top * BLOCK_SIZE,
                (z * BLOCK_SIZE) - (BLOCK_SIZE >> 1)
            );
            vertices[2] = vec3_i16(
                x * BLOCK_SIZE,
                -y_bottom * BLOCK_SIZE,
                (z * BLOCK_SIZE) - (BLOCK_SIZE >> 1)
            );
            vertices[3] = vec3_i16(
                (x + 1) * BLOCK_SIZE,
                -y_bottom * BLOCK_SIZE,
                (z * BLOCK_SIZE) - (BLOCK_SIZE >> 1)
            );
            POLY_FT4* pol4 = createQuad(
                vertices,
                FACE_DIRECTION_NORMALS[x < player_pos.vx ? FACE_DIR_RIGHT : FACE_DIR_LEFT],
                ctx,
                &p
            );
            if (pol4 != NULL) {
                setUVWH(
                    pol4,
                    u_rand_offset,
                    v_rand_offset,
                    WEATHER_TEXTURE_WIDTH,
                    WEATHER_TEXTURE_HEIGHT
                );
                // Bind texture page and colour look-up-table
                pol4->tpage = texture->tpage;
                pol4->clut = texture->clut;
                setSemiTrans(pol4, 0);
                // Sort primitive to the ordering table
                const u32* ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, pol4);
                const RECT tex_window = (RECT) {
                    .w = WEATHER_TEXTURE_WIDTH >> 3,
                    .h = WEATHER_TEXTURE_HEIGHT >> 3,
                    // Use the second half of the texture for snow
                    .x = current_is_snow ? (WEATHER_TEXTURE_WIDTH >> 3) : 0,
                    .y = 0
                };
                DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
                setTexWindow(ptwin, &tex_window);
                ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, ptwin);
            }
            // Z-axis
            vertices[0] = vec3_i16(
                (x * BLOCK_SIZE) - (BLOCK_SIZE >> 1),
                -y_top * BLOCK_SIZE,
                z * BLOCK_SIZE
            );
            vertices[1] = vec3_i16(
                (x * BLOCK_SIZE) - (BLOCK_SIZE >> 1),
                -y_top * BLOCK_SIZE,
                (z + 1) * BLOCK_SIZE
            );
            vertices[2] = vec3_i16(
                (x * BLOCK_SIZE) - (BLOCK_SIZE >> 1),
                -y_bottom * BLOCK_SIZE,
                z * BLOCK_SIZE
            );
            vertices[3] = vec3_i16(
                (x * BLOCK_SIZE) - (BLOCK_SIZE >> 1),
                -y_bottom * BLOCK_SIZE,
                (z + 1) * BLOCK_SIZE
            );
            pol4 = createQuad(
                vertices,
                FACE_DIRECTION_NORMALS[z < player_pos.vz ? FACE_DIR_BACK : FACE_DIR_FRONT],
                ctx,
                &p
            );
            if (pol4 != NULL) {
                setUVWH(
                    pol4,
                    u_rand_offset,
                    v_rand_offset,
                    WEATHER_TEXTURE_WIDTH,
                    WEATHER_TEXTURE_HEIGHT
                );
                // Bind texture page and colour look-up-table
                pol4->tpage = texture->tpage;
                pol4->clut = texture->clut;
                // Sort primitive to the ordering table
                u32* ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, pol4);
                const RECT tex_window = (RECT) {
                    .w = WEATHER_TEXTURE_WIDTH >> 3,
                    .h = WEATHER_TEXTURE_HEIGHT >> 3,
                    // Use the second half of the texture for snow
                    .x = current_is_snow ? (WEATHER_TEXTURE_WIDTH >> 3) : 0,
                    .y = 0
                };
                DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
                setTexWindow(ptwin, &tex_window);
                ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, ptwin);
            }
        }
    }
    render_ticks = (render_ticks + 1) % WEATHER_TEXTURE_HEIGHT;
    // Restore matrix
    PopMatrix();
}

