#include "weather.h"

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../util/preprocessor.h"
#include "../world/position.h"
#include "../../structure/primitive/clip.h"
#include "../../structure/primitive/direction.h"
#include "../../math/math_utils.h"
#include "../../logging/logging.h"

FWD_DECL ChunkHeightmap* worldGetChunkHeightmap(World* world, const VECTOR* position);

static POLY_FT4* createQuad(const SVECTOR vertices[4],
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
    if (quadClip(
        &ctx->screen_clip,
        (DVECTOR*) &pol4->x0,
        (DVECTOR*) &pol4->x1,
        (DVECTOR*) &pol4->x2,
        (DVECTOR*) &pol4->x3)) {
        freePrimitive(ctx, sizeof(POLY_FT4));
        return NULL;
    }
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
        player->entity.physics_object.position,
        ONE_BLOCK
    );
    ChunkBlockPosition cb_pos;
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__WEATHER];
    int p = 0;
    renderCtxBindMatrix(
        ctx,
        transforms,
        &VEC3_I16_ZERO,
        &VEC3_I32_ZERO
    );
    for (i32 x = player_pos.vx - WEATHER_RENDER_RADIUS; x <= player_pos.vx + WEATHER_RENDER_RADIUS; x++) {
        for (i32 z = player_pos.vz - WEATHER_RENDER_RADIUS; z <= player_pos.vz + WEATHER_RENDER_RADIUS; z++) {
            const VECTOR pos = vec3_i32(x, 0, z);
            cb_pos = worldToChunkBlockPosition(&pos, CHUNK_SIZE);
            ChunkHeightmap* heightmap = worldGetChunkHeightmap((World*) world, &cb_pos.chunk);
            const i32 top_solid_block_y = (*heightmap)[chunkHeightmapIndex(
                cb_pos.block.vx,
                cb_pos.block.vz
            )] + 1;
            // Extra -1 here is to make the radius start from the
            // player's head and feet, to make it feel uniform
            // in down direction
            const i32 y_bottom = max(player_pos.vy - WEATHER_RENDER_RADIUS - 1, top_solid_block_y);
            const i32 y_top = max(player_pos.vy + WEATHER_RENDER_RADIUS, top_solid_block_y);
            if (y_bottom == y_top) {
                continue;
            }
            const bool current_is_snow = false; // TODO: Set this to true if we are in a tiaga biome
            // UV offsets
            srand(x * x * 3121 + x * 45238971 + z * z * 418711 + z * 13761);
            // FIXME: The scrolling texture for weather doesnt work for some reason
            const u32 u_rand_offset = positiveModulo(rand(), WEATHER_TEXTURE_HALF_WIDTH);
            const u32 v_rand_offset = positiveModulo(positiveModulo(rand(), WEATHER_TEXTURE_HEIGHT) + render_ticks - (BLOCK_SIZE * y_top), WEATHER_TEXTURE_HEIGHT);
            const bool x_dir = x >= player_pos.vx;
            SVECTOR vertices[4];
            // X-axis
            vertices[(0 + x_dir) % 2] = vec3_i16(
                x * BLOCK_SIZE,
                -y_top * BLOCK_SIZE,
                (z * BLOCK_SIZE) + (BLOCK_SIZE >> 1)
            );
            vertices[(1 + x_dir) % 2] = vec3_i16(
                (x + 1) * BLOCK_SIZE,
                -y_top * BLOCK_SIZE,
                (z * BLOCK_SIZE) + (BLOCK_SIZE >> 1)
            );
            vertices[2 + ((0 + x_dir) % 2)] = vec3_i16(
                x * BLOCK_SIZE,
                -y_bottom * BLOCK_SIZE,
                (z * BLOCK_SIZE) + (BLOCK_SIZE >> 1)
            );
            vertices[2 + ((1 + x_dir) % 2)] = vec3_i16(
                (x + 1) * BLOCK_SIZE,
                -y_bottom * BLOCK_SIZE,
                (z * BLOCK_SIZE) + (BLOCK_SIZE >> 1)
            );
            POLY_FT4* pol4 = createQuad(
                vertices,
                FACE_DIRECTION_NORMALS[FACE_DIR_RIGHT + x_dir],
                ctx,
                &p
            );
            if (pol4 != NULL) {
                setUVWH(
                    pol4,
                    u_rand_offset,
                    v_rand_offset,
                    BLOCK_SIZE,
                    BLOCK_SIZE * (y_top - y_bottom)
                );
                // Bind texture page and colour look-up-table
                pol4->tpage = texture->tpage;
                pol4->clut = texture->clut;
                // Sort primitive to the ordering table
                const u32* ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, pol4);
                const RECT tex_window = (RECT) {
                    .w = WEATHER_TEXTURE_HALF_WIDTH >> 3,
                    .h = WEATHER_TEXTURE_HEIGHT >> 3,
                    // Use the second half of the texture for snow
                    .x = current_is_snow ? (WEATHER_TEXTURE_HALF_WIDTH >> 3) : 0,
                    .y = 0 
                };
                DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
                setTexWindow(ptwin, &tex_window);
                ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, ptwin);
            }
            // Z-axis
            const bool z_dir = z >= player_pos.vz;
            vertices[(0 + z_dir) % 2] = vec3_i16(
                (x * BLOCK_SIZE) + (BLOCK_SIZE >> 1),
                -y_top * BLOCK_SIZE,
                z * BLOCK_SIZE
            );
            vertices[(1 + z_dir) % 2] = vec3_i16(
                (x * BLOCK_SIZE) + (BLOCK_SIZE >> 1),
                -y_top * BLOCK_SIZE,
                (z + 1) * BLOCK_SIZE
            );
            vertices[2 + ((0 + z_dir) % 2)] = vec3_i16(
                (x * BLOCK_SIZE) + (BLOCK_SIZE >> 1),
                -y_bottom * BLOCK_SIZE,
                z * BLOCK_SIZE
            );
            vertices[2 + ((1 + z_dir) % 2)] = vec3_i16(
                (x * BLOCK_SIZE) + (BLOCK_SIZE >> 1),
                -y_bottom * BLOCK_SIZE,
                (z + 1) * BLOCK_SIZE
            );
            pol4 = createQuad(
                vertices,
                FACE_DIRECTION_NORMALS[FACE_DIR_BACK + z_dir],
                ctx,
                &p
            );
            if (pol4 != NULL) {
                setUVWH(
                    pol4,
                    u_rand_offset,
                    v_rand_offset,
                    BLOCK_SIZE,
                    BLOCK_SIZE * (y_top - y_bottom)
                );
                // Bind texture page and colour look-up-table
                pol4->tpage = texture->tpage;
                pol4->clut = texture->clut;
                // Sort primitive to the ordering table
                u32* ot_object = allocateOrderingTable(ctx, p);
                addPrim(ot_object, pol4);
                const RECT tex_window = (RECT) {
                    .w = WEATHER_TEXTURE_HALF_WIDTH >> 3,
                    .h = WEATHER_TEXTURE_HEIGHT >> 3,
                    // Use the second half of the texture for snow
                    .x = current_is_snow ? (WEATHER_TEXTURE_HALF_WIDTH >> 3) : 0,
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
    renderCtxUnbindMatrix();
}

