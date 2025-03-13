#include "world.h"

#include <assert.h>
#include <psxapi.h>
#include <psxgte.h>
#include <inline_c.h>
#include <psxgpu.h>

#include "../../util/interface99_extensions.h"
#include "../../structure/primitive/clip.h"
#include "../../render/duration_tree.h"
#include "../../render/font.h"
#include "../../math/math_utils.h"
#include "../../math/vector.h"
#include "../../ui/progress_bar.h"
#include "../../ui/background.h"
#include "../../logging/logging.h"
#include "../items/items.h"
#include "chunk/chunk.h"
#include "chunk/chunk_defines.h"
#include "chunk/chunk_mesh.h"
#include "chunk/chunk_structure.h"
#include "chunk/heightmap.h"
#include "position.h"
#include "world_defines.h"

const LightUpdateLimits world_chunk_init_limits = (LightUpdateLimits) {
    .add_block = 0,
    .add_sky = 0,
    .remove_block = 0,
    .remove_sky = 0
};

// NOTE: Cast to i32 is necessary here since computing modulo of 0 - 1
//       is actually computing modulo over 0u32 - 1 == u32::MAX so we end
//       up with a result of 0. This means that you can never move in a
//       negative direction. Casting to i32 fixes this to allow for wrapping
#define wrapCoord(world, axis, coord) positiveModulo((((i32)(world)->head.axis) + (coord)), AXIS_CHUNKS)
#define arrayCoord(world, axis, value) wrapCoord(\
    world, \
    axis, \
    ((value) - (world->centre.axis - LOADED_CHUNKS_RADIUS - SHIFT_ZONE))\
)

static void displayProgress(RenderContext* ctx,
                            ProgressBar* progress_bar,
                            const i32 x,
                            const i32 y,
                            const i32 z,
                            const char* msg) {
    fontPrintCentreOffset(
        ctx,
        CENTRE_X,
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 3),
        0,
        "Loading World"
    );
    fontPrintCentreOffset(
        ctx,
        CENTRE_X,
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 2) - 1,
        10,
        "Chunk [%d,%d,%d]",
        x, y, z
    );
    fontPrintCentreOffset(
        ctx,
        CENTRE_X,
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 1) - 1,
        10,
        msg,
        x, y, z
    );
    const u32* background_ot_object = allocateOrderingTable(ctx, 2);
    backgroundDraw(
        ctx,
        background_ot_object,
        2 * BLOCK_TEXTURE_SIZE,
        0 * BLOCK_TEXTURE_SIZE
    );
    progress_bar->value++;
    progressBarRender(progress_bar, 1, ctx);
    swapBuffers(ctx);
}

// World should be loaded before invoking this method
void worldInit(World* world, RenderContext* ctx) {
    world->internal_light_level = createLightLevel(0, 15);
    world->time_ticks = WORLD_TIME_DAWN;
    world->day_count = 0;
    world->celestial_angle = 0;
    world->weather = (Weather) {
        .rain_strength = 0,
        .storm_strength = 0,
        .rain_time_ticks = 0,
        .storm_time_ticks = 0,
        .raining = false,
        .storming = false
    };
    memset(
        world->heightmap,
        '\0',
        sizeof(u32) * AXIS_CHUNKS * AXIS_CHUNKS * CHUNK_SIZE * CHUNK_SIZE
    );
    VCALL(world->chunk_provider, init);
    // Clear the chunks first to ensure they are all NULL upon initialisation
    memset(
        world->chunks,
        0,
        sizeof(Chunk*) * WORLD_CHUNKS_COUNT
    );
    ChunkGenerationContext gen_ctx[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT] = {0};
    const i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const i32 x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    ProgressBar bar = (ProgressBar) {
        .position = {
            .x = CENTRE_X - (CENTRE_X / 3),
            .y = CENTRE_Y - 2
        },
        .dimensions = {
            .width = (2 * CENTRE_X / 3),
            .height = 4
        },
        .value = 0,
        .maximum = ((x_end + 1) - x_start) * ((z_end + 1) - z_start) * WORLD_CHUNKS_HEIGHT * 5
    };
    DEBUG_LOG("[WORLD] Loading chunks\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                Chunk* chunk = worldLoadChunk(
                    world,
                    vec3_i32(x,y,z)
                );
                world->chunks[arrayCoord(world, vz, z)]
                             [arrayCoord(world, vx, x)]
                             [y] = chunk;
                displayProgress(ctx, &bar, x, y, z, "Loading Chunk Data");
            }
        }
    }
    DEBUG_LOG("[WORLD] Generating Lightmaps\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("[CHUNK: %d,%d,%d] Generating Lightmap\n", x, 0, z);
                const u16 array_x = arrayCoord(world, vx, x);
                const u16 array_z = arrayCoord(world, vz, z);
                Chunk* chunk = world->chunks[array_z][array_x][y];
                chunkGenerateLightmap(chunk, &gen_ctx[array_z][array_z][y]);
                displayProgress(ctx, &bar, x, y, z, "Generating Lightmap");
            }
        }
    }
    DEBUG_LOG("[WORLD] Propagating Light\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("[CHUNK: %d,%d,%d] Propagating Light\n", x, 0, z);
                const u16 array_x = arrayCoord(world, vx, x);
                const u16 array_z = arrayCoord(world, vz, z);
                Chunk* chunk = world->chunks[array_z][array_x][y];
                chunkPropagateLightmap(chunk, &gen_ctx[array_z][array_x][y]);
                displayProgress(ctx, &bar, x, y, z, "Propagating Light");
            }
        }
    }
    DEBUG_LOG("[WORLD] Processing Light Updates\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("[CHUNK: %d,%d,%d] Processing Light Updates\n", x, 0, z);
           Chunk* chunk = world->chunks[arrayCoord(world, vz, z)]
                                            [arrayCoord(world, vx, x)]
                                            [y];
                chunkUpdateLight(chunk, world_chunk_init_limits);
                displayProgress(ctx, &bar, x, y, z, "Processing Light Updates");
            }
        }
    }
    DEBUG_LOG("[WORLD] Building chunk meshes\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("[CHUNK: %d,%d,%d] Generating mesh\n", x, 0, z);
                Chunk* chunk = world->chunks[arrayCoord(world, vz, z)]
                                            [arrayCoord(world, vx, x)]
                                            [y];
                chunkGenerateMesh(chunk);
                #define layoutMeshAttrs(attribute) \
                    chunk->mesh.face_meshes[FACE_DIR_DOWN].attribute, \
                    chunk->mesh.face_meshes[FACE_DIR_UP].attribute, \
                    chunk->mesh.face_meshes[FACE_DIR_LEFT].attribute, \
                    chunk->mesh.face_meshes[FACE_DIR_RIGHT].attribute, \
                    chunk->mesh.face_meshes[FACE_DIR_BACK].attribute, \
                    chunk->mesh.face_meshes[FACE_DIR_FRONT].attribute
                DEBUG_LOG(
                    "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Mesh {\n"
                    "   Primitives: [%d,%d,%d,%d,%d,%d]\n"
                    "   Vertices: [%d,%d,%d,%d,%d,%d]\n"
                    "   Normals: [%d,%d,%d,%d,%d,%d]\n"
                    "}\n",
                    x, y, z,
                    arrayCoord(world, vx, x),
                    y,
                    arrayCoord(world, vz, z),
                    layoutMeshAttrs(n_prims),
                    layoutMeshAttrs(n_verts),
                    layoutMeshAttrs(n_norms)
                );
                #undef layoutMeshAttrs
                displayProgress(ctx, &bar, x, y, z, "Building Mesh");
            }
        }
    }
#undef displayProgress
    DEBUG_LOG("[WORLD] Finished loading\n");
}

void worldDestroy(World* world) {
    const i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const i32 x_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    const i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG(
                    "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Destroying chunk\n",
                    x, y, z,
                    arrayCoord(world, vx, x),
                    y,
                    arrayCoord(world, vz, z)
                );
                Chunk** chunk = world->chunks[arrayCoord(world, vz, z)]
                                             [arrayCoord(world, vx, x)];
                worldUnloadChunk(world, chunk[y]);
                chunk[y] = NULL;
            }
        }
    }
    VCALL(world->chunk_provider, destroy);
    free(world->chunk_provider.self);
}

#define SUN_DISTANCE 1000
#define SUN_SIZE 10

static const VECTOR sun_vertices[4] = {
    [0]=vec3_i32(SUN_DISTANCE, -SUN_SIZE, -SUN_SIZE),
    [1]=vec3_i32(SUN_DISTANCE, -SUN_SIZE, SUN_SIZE),
    [2]=vec3_i32(SUN_DISTANCE, SUN_SIZE, -SUN_SIZE),
    [3]=vec3_i32(SUN_DISTANCE, SUN_SIZE, SUN_SIZE)
};

void worldRenderSkybox(const World* world,
                       const VECTOR* player_world_pos,
                       RenderContext* ctx,
                       Transforms* transforms) {
    renderClearConstraintsIndex(ctx, ORDERING_TABLE_LENGTH - 1);
    const SVECTOR rotation = vec3_i16(0, 0, world->celestial_angle);
    renderCtxBindMatrix(
        ctx,
        transforms,
        &rotation,
        player_world_pos
    );
    POLY_FT4* sun = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    const u32* ot_object = allocateOrderingTable(ctx, ORDERING_TABLE_LENGTH - 1);
    setPolyFT4(sun);
    gte_ldv3(
        &sun_vertices[0],
        &sun_vertices[1],
        &sun_vertices[2]
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    gte_nclip();
    int p;
    gte_stopz(&p);
    // gte_stdp(&dp);
    // Avoid negative depth (behind camera) and zero
    // for constraint clearing primitive in OT
    if (p < 0) {
        goto render_moon;
    }
    // Initialize a textured quad primitive
    setPolyFT4(sun);
    // Set the projected vertices to the primitive
    gte_stsxy0(&sun->x0);
    gte_stsxy1(&sun->x1);
    gte_stsxy2(&sun->x2);
    // Compute the last vertex and set the result
    gte_ldv0(&sun_vertices[3]);
    gte_rtps();
    gte_stsxy(&sun->x3);
    // Test if quad is off-screen, discard if so
    if (quadClip(
        &ctx->screen_clip,
        (DVECTOR*) &sun->x0,
        (DVECTOR*) &sun->x1,
        (DVECTOR*) &sun->x2,
        (DVECTOR*) &sun->x3)) {
        goto render_moon;
    }
    setRGB0(sun, 0xFF, 0xFF, 0xFF);
    setUVWH(sun, 0, 0, 32, 32);
    // TODO: Include sun texture in assets
    /*const Texture* texture = &textures[ASSET_TEXTURE__STATIC__SUN];*/
    /*sun->tpage = texture->tpage;*/
    /*sun->clut = texture->clut;*/
    addPrim(ot_object, sun);
    renderCtxUnbindMatrix();
    return;
render_moon:;
    // TODO: Render moon
    freePrimitive(ctx, sizeof(POLY_FT4));
    renderCtxUnbindMatrix();
}

DEFN_DURATION_COMPONENT(world_render);

void worldRender(const World* world,
                 const Player* player,
                 RenderContext* ctx,
                 Transforms* transforms) {
    durationComponentInitOnce(world_render, "worldRender");
    durationComponentStart(world_render_duration);
    const VECTOR player_world_pos = vec3_i32(
        fixedFloor(player->entity.physics_object.position.vx, ONE_BLOCK) / ONE_BLOCK,
        fixedFloor(player->entity.physics_object.position.vy, ONE_BLOCK) / ONE_BLOCK,
        fixedFloor(player->entity.physics_object.position.vz, ONE_BLOCK) / ONE_BLOCK
    );
    worldRenderSkybox(world, &player_world_pos, ctx, transforms);
    const VECTOR player_chunk_pos = worldToChunkBlockPosition(
        &player_world_pos,
        CHUNK_SIZE
    ).chunk;
    const i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const i32 x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    // TODO: Render current chunk and track how much of the screen has been drawn (somehow?)
    //       if there are still bits that are missing traverse to next chunks in the direction
    //       the player is facing and render them. Stop drawing if screen is full and/or there
    //       are no more loaded chunks to traverse to.
    //
    // PERF: Refactor to use ChunkVisibility to render based on DFS
    //       that queries visibility of exiting the current chunk.
    //       Also need to consider if the chunk is outside the FOV
    //       angle width-wise and stop searching in that direction if
    //       so.
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                chunkRender(
                    world->chunks[arrayCoord(world, vz, z)]
                                 [arrayCoord(world, vx, x)]
                                 [y],
                    player_chunk_pos.vx == x
                        && player_chunk_pos.vz == z
                        && player_chunk_pos.vy == y,
                    ctx,
                    transforms
                );
            }
        }
    }
    durationComponentEnd();
    durationTreeRender(world_render_duration, ctx, transforms);
}

// NOTE: Should this just take i32 x,y,z params instead of a
//       a VECTOR struct to avoid creating needless stack objects?
Chunk* worldLoadChunk(World* world, const VECTOR chunk_position) {
    Chunk* chunk = VCALL(
        world->chunk_provider,
        provide,
        chunk_position,
        &world->heightmap[arrayCoord(world, vz, chunk_position.vz)]
                         [arrayCoord(world, vx, chunk_position.vx)]
    );
    assert(chunk != NULL);
    // DEBUG_LOG(
    //     "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Generating heightmap and terrain\n",
    //     chunk_position.vx,
    //     chunk_position.vy,
    //     chunk_position.vz,
    //     arrayCoord(world, vx, chunk_position.vx),
    //     chunk_position.vy,
    //     arrayCoord(world, vz, chunk_position.vz)
    // );
    chunk->world = world;
    return chunk;
}

void worldUnloadChunk(const World* world, Chunk* chunk) {
    // DEBUG_LOG(
    //     "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Unloading chunk\n",
    //     chunk->position.vx,
    //     chunk->position.vy,
    //     chunk->position.vz,
    //     arrayCoord(world, vx, chunk->position.vx),
    //     chunk->position.vy,
    //     arrayCoord(world, vz, chunk->position.vz)
    // );
    VCALL(world->chunk_provider, save, chunk);
    free(chunk);
}

void worldLoadChunksX(World* world, const i8 x_direction, const i8 z_direction) {
    // Load x_direction chunks
    i32 x_shift_zone = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    if (z_direction == -1) {
        z_end -= SHIFT_ZONE;
    } else if (z_direction == 1) {
        z_start += SHIFT_ZONE;
    }
    Chunk* to_mesh[(z_end + 1 - z_start) * WORLD_CHUNKS_HEIGHT];
    ChunkGenerationContext gen_ctx[(z_end + 1 - z_start) * WORLD_CHUNKS_HEIGHT];
    u32 i = 0;
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, vec3_i32(x_shift_zone, y, z_coord));
            world->chunks[arrayCoord(world, vz, z_coord)]
                         [arrayCoord(world, vx, x_shift_zone)]
                         [y] = to_mesh[i++] = chunk;
        }
    }
    i = 0;
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkGenerateLightmap(to_mesh[i], &gen_ctx[i]);
            i++;
        }
    }
    i = 0;
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkPropagateLightmap(to_mesh[i], &gen_ctx[i]);
            i++;
        }
    }
    // We need to generate the mesh after creating all the chunks
    // since each chunk needs to index into it's neighbours to
    // determine if a mesh face should be created on the border.
    // So if the neighbour doesn't exit yet we get more quads in
    // a chunk mesh than are actually necessary.
    i = 0;
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkGenerateMesh(to_mesh[i++]);
        }
    }
    // Unload -x_direction chunks
    x_shift_zone = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        Chunk** chunk = world->chunks[arrayCoord(world, vz, z_coord)]
                                     [arrayCoord(world, vx, x_shift_zone)];
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            worldUnloadChunk(world, chunk[y]);
            chunk[y] = NULL;
        }
    }
}

void worldLoadChunksZ(World* world, const i8 x_direction, const i8 z_direction) {
    // Load z_direction chunks
    i32 z_shift_zone = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;;
    i32 x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    /* Can be simplified to:
     * i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS + ((x_direction == -1) * SHIFT_ZONE);
     * i32 x_end = world->centre.vz + LOADED_CHUNKS_RADIUS - ((x_direction == 1) * SHIFT_ZONE);
     */
    if (x_direction == -1) {
        x_end -= SHIFT_ZONE;
    } else if (x_direction == 1) {
        x_start += SHIFT_ZONE;
    }
    Chunk* to_mesh[(x_end + 1 - x_start) * WORLD_CHUNKS_HEIGHT];
    ChunkGenerationContext gen_ctx[(x_end + 1 - x_start) * WORLD_CHUNKS_HEIGHT];
    u32 i = 0;
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, vec3_i32(x_coord, y, z_shift_zone));
            world->chunks[arrayCoord(world, vz, z_shift_zone)]
                         [arrayCoord(world, vx, x_coord)]
                         [y] = to_mesh[i++] = chunk;
        }
    }
    i = 0;
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkGenerateLightmap(to_mesh[i], &gen_ctx[i]);
            i++;
        }
    }
    i = 0;
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkPropagateLightmap(to_mesh[i], &gen_ctx[i]);
            i++;
        }
    }
    // We need to generate the mesh after creating all the chunks
    // since each chunk needs to index into it's neighbours to
    // determine if a mesh face should be created on the border.
    // So if the neighbour doesn't exit yet we get more quads in
    // a chunk mesh than are actually necessary.
    i = 0;
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkGenerateMesh(to_mesh[i++]);
        }
    }
    // Unload -z_direction chunks
    z_shift_zone = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        Chunk** chunk = world->chunks[arrayCoord(world, vz, z_shift_zone)]
                                     [arrayCoord(world, vx, x_coord)];
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            worldUnloadChunk(world, chunk[y]);
            chunk[y] = NULL;
        }
    }
}

void worldLoadChunksXZ(World* world, const i8 x_direction, const i8 z_direction) {
    // Load (x_direction,z_direction) chunk
    i32 x_coord = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    i32 z_coord = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    Chunk* to_mesh[WORLD_CHUNKS_HEIGHT] = {0};
    ChunkGenerationContext gen_ctx[WORLD_CHUNKS_HEIGHT] = {0};
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk* loaded_chunk = worldLoadChunk(world, vec3_i32(x_coord, y, z_coord));
        world->chunks[arrayCoord(world, vz, z_coord)]
                     [arrayCoord(world, vx, x_coord)]
                     [y] = to_mesh[y] = loaded_chunk;
    }
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        chunkGenerateLightmap(to_mesh[y], &gen_ctx[y]);
    }
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        chunkPropagateLightmap(to_mesh[y], &gen_ctx[y]);
    }
    // We need to generate the mesh after creating all the chunks
    // since each chunk needs to index into it's neighbours to
    // determine if a mesh face should be created on the border.
    // So if the neighbour doesn't exit yet we get more quads in
    // a chunk mesh than are actually necessary.
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        // TODO: Should these mesh generations be done for all axis after we
        //       have loaded chunks on all axis? Otherwise each axis may duplicate
        //       faces from, the orthogonal axis.
        chunkGenerateMesh(to_mesh[y]);
    }
    // Unload (-x_direction,-z_direction) chunk
    x_coord = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    z_coord = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    Chunk** unloaded_chunk = world->chunks[arrayCoord(world, vz, z_coord)]
                                          [arrayCoord(world, vx, x_coord)];
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        worldUnloadChunk(world, unloaded_chunk[y]);
        unloaded_chunk[y] = NULL;
    }
}

INLINE bool worldPlayerWithinLoadRadius(const World* world, const VECTOR* player_chunk_pos) {
    return absv(world->centre.vx - player_chunk_pos->vx) < LOADED_CHUNKS_RADIUS - 1
           && absv(world->centre.vz - player_chunk_pos->vz) < LOADED_CHUNKS_RADIUS - 1;
}

void worldLoadChunks(World* world, const VECTOR* player_chunk_pos) {
    // Check if we need to load
    if (worldPlayerWithinLoadRadius(world, player_chunk_pos)) {
        return;
    }
    // Calculate direction shifts
    const i8 x_direction = cmp(world->centre.vx, player_chunk_pos->vx);
    const i8 z_direction = cmp(world->centre.vz, player_chunk_pos->vz);
    // Shift chunks into centre of arrays
    world->centre_next.vx += x_direction;
    world->centre_next.vz += z_direction;
    // Load chunks
    if (x_direction != 0) {
        DEBUG_LOG("[WORLD] Load chunks X\n");
        worldLoadChunksX(world, x_direction, z_direction);
        DEBUG_LOG("[WORLD] Load chunks X ... done\n");
    }
    if (z_direction != 0) {
        DEBUG_LOG("[WORLD] Load chunks Z\n");
        worldLoadChunksZ(world, x_direction, z_direction);
        DEBUG_LOG("[WORLD] Load chunks Z ... done\n");
    }
    if (x_direction != 0 && z_direction != 0) {
        DEBUG_LOG("[WORLD] Load chunks XZ\n");
        worldLoadChunksXZ(world, x_direction, z_direction);
        DEBUG_LOG("[WORLD] Load chunks XZ ... done\n");
    }
    // Synchronise centre
    world->head.vx = wrapCoord(world, vx, x_direction);
    world->head.vz = wrapCoord(world, vz, z_direction);
    world->centre = world->centre_next;
}

bool isPlayerInEdgeChunks(const World* world, const ChunkBlockPosition* player_pos) {
    static i32 prev_pos_chunk_y = 0;
#define inChunkEdge(axis, last) (player_pos->block.axis == \
    ((player_pos->chunk.axis - world->centre.axis >= 0) * (last)))
#define inEdge(axis, delta) (absv(player_pos->chunk.axis - world->centre.axis) == (delta))
    const bool result = (inEdge(vx, LOADED_CHUNKS_RADIUS) && inChunkEdge(vx, CHUNK_SIZE - 1))
        || (inEdge(vz, LOADED_CHUNKS_RADIUS) && inChunkEdge(vz, CHUNK_SIZE - 1))
        || prev_pos_chunk_y != player_pos->chunk.vy;
#undef inEdge
    prev_pos_chunk_y = player_pos->chunk.vy;
    return result;
}

fixedi32 calculateCelestialAngle(u16 time_ticks) {
    fixedi32 scaled = (((time_ticks + 1) << FIXED_POINT_SHIFT) / WORLD_TIME_CYCLE) - FIXED_1_4;
    if (scaled < 0) {
        scaled += ONE;
    }
    if (scaled > ONE) {
        scaled -= ONE;
    }
    fixedi32 cached_scaled = scaled;
    scaled = ONE - ((cos5o(fixedMul(scaled, FIXED_PI)) + ONE) >> 1);
    scaled = cached_scaled + ((scaled - cached_scaled) / 3);
    return scaled;
}

static LightLevel previous_light_level = createLightLevel(0, 0);

void worldUpdateInternalLightLevel(World* world, RenderContext* ctx) {
    const fixedi32 celestial_angle = calculateCelestialAngle(world->time_ticks);
    fixedi32 scaled = ONE - ((cos5o(fixedMul(celestial_angle, FIXED_PI << 1)) << 1) + FIXED_1_2);
    scaled = ONE - clamp(scaled, 0, ONE);
    scaled = fixedMul(scaled, ONE - (((fixedi32) world->weather.rain_strength * 5) >> 4)); // Same as div 16
    scaled = fixedMul(scaled, ONE - (((fixedi32) world->weather.storm_strength * 5) >> 4));
    scaled = ONE - scaled;
    previous_light_level = world->internal_light_level;
    world->internal_light_level = createLightLevel(0, 15 - ((scaled * 11) >> FIXED_POINT_SHIFT));
    world->celestial_angle = celestial_angle;
    if (previous_light_level == world->internal_light_level) {
        return;
    }
    const u16 light_level_colour_scalar = lightLevelColourScalar(
        world->internal_light_level,
        createLightLevel(0, 15)
    );
    const u8 colour = fixedMul(0x7F, light_level_colour_scalar);
    const CVECTOR ambient_colour = vec3_rgb_all(colour);
    // TODO: Tint the colour 
    gte_SetBackColor(
        ambient_colour.r,
        ambient_colour.g,
        ambient_colour.b
    );
    gte_SetFarColor(
        ambient_colour.r,
        ambient_colour.g,
        ambient_colour.b
    );
    setRGB0(
        &ctx->db[0].draw_env,
        ambient_colour.r,
        ambient_colour.g,
        ambient_colour.b
    );
    setRGB0(
        &ctx->db[1].draw_env,
        ambient_colour.r,
        ambient_colour.g,
        ambient_colour.b
    );
}

// See: https://minecraft.wiki/w/Light#Internal_light_level
void worldUpdateInternalLightLevelOld(World* world) {
    LightLevel internal_light_level = createLightLevel(0, 15);
    #define setLevel(level) internal_light_level = createLightLevel(0, level)
    switch (world->time_ticks) {
        case 13670 ... 22330:
            setLevel(4);
            break;
        case 22331 ... 22491:
        case 13509 ... 13669:
            setLevel(5);
            break;
        case 22492 ... 22652:
        case 13348 ... 13508:
            setLevel(6);
            break;
        case 22653 ... 22812:
        case 13188 ... 13347:
            setLevel(7);
            break;
        case 22813 ... 22973:
        case 13027 ... 13187:
            setLevel(8);
            break;
        case 22974 ... 23134:
        case 12867 ... 13026:
            setLevel(9);
            break;
        case 23135 ... 23296:
        case 12705 ... 12866:
            setLevel(10);
            break;
        case 23297 ... 23459:
        case 12542 ... 12704:
            setLevel(11);
            break;
        case 23460 ... 23623:
        case 12377 ... 12541:
            setLevel(12);
            break;
        case 23624 ... 23790:
        case 12210 ... 12376:
            setLevel(13);
            break;
        case 23791 ... 23960:
        case 12041 ... 12209:
            setLevel(14);
            break;
        case 23961 ... 24000:
        case 0 ... 12040:
            setLevel(15);
            break;
    }
    world->internal_light_level = internal_light_level;
}

void worldUpdateWeather(Weather* weather) {
    if (weather->storm_time_ticks <= 0) {
        if (weather->storm_strength > 0) {
            weather->storm_time_ticks = (rand() % 12000) + 3600;
        } else {
            weather->storm_time_ticks = (rand() % 16800) + 12000;
        }
    } else {
        if (--weather->storm_time_ticks <= 0) {
            weather->storming = !weather->storming;
        }
    }
    const fixedi16 storm_strength = weather->storm_strength
        + (weather->storming ? WEATHER_STRENGTH_INCREMENT : -WEATHER_STRENGTH_INCREMENT);
    weather->storm_strength = clamp(storm_strength, 0, ONE);
    if (weather->rain_time_ticks <= 0) {
        if (weather->rain_strength > 0) {
            weather->rain_time_ticks = (rand() % 12000) + 12000;
        } else {
            weather->rain_time_ticks = (rand() % 16800) + 12000;
        }
    } else {
        if (--weather->rain_time_ticks <= 0) {
            weather->raining = !weather->raining;
        }
    }
    const fixedi16 rain_strength = weather->rain_strength
        + (weather->raining ? WEATHER_STRENGTH_INCREMENT : -WEATHER_STRENGTH_INCREMENT);
    weather->rain_strength = clamp(rain_strength, 0, ONE);
}

void worldUpdate(World* world,
                 Player* player,
                 BreakingState* breaking_state,
                 RenderContext* ctx) {
    worldUpdateWeather(&world->weather);
    worldUpdateInternalLightLevel(world, ctx);
    world->time_ticks = positiveModulo(world->time_ticks + 1, WORLD_TIME_CYCLE);
    const VECTOR player_world_pos = vec3_i32(
        fixedFloor(player->entity.physics_object.position.vx, ONE_BLOCK) / ONE_BLOCK,
        fixedFloor(player->entity.physics_object.position.vy, ONE_BLOCK) / ONE_BLOCK,
        fixedFloor(player->entity.physics_object.position.vz, ONE_BLOCK) / ONE_BLOCK
    );
    const ChunkBlockPosition player_pos = worldToChunkBlockPosition(
        &player_world_pos,
        CHUNK_SIZE
    );
    if (isPlayerInEdgeChunks(world, &player_pos)) {
        DEBUG_LOG("Player chunk pos: " VEC_PATTERN "\n", VEC_LAYOUT(player_pos.chunk));
        worldLoadChunks(world, &player_pos.chunk);
        DEBUG_LOG(
            "[WORLD] Head { x: %d, z: %d } Centre { x: %d, z: %d}\n",
            world->head.vx, world->head.vz,
            world->centre.vx, world->centre.vz
        );
        for (i32 z = 0; z < AXIS_CHUNKS; z++) {
            for (i32 x = 0; x < AXIS_CHUNKS; x++) {
                DEBUG_LOG("%d ", world->chunks[z][x][0] != NULL);
            }
            DEBUG_LOG("\n");
        }
    }
    const VECTOR chunk_position = worldToChunkBlockPosition(
        &breaking_state->position,
        CHUNK_SIZE
    ).chunk;
    const i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const i32 x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    if (breaking_state->block == NULL) {
        // Optimised to skip coords checks when we aren't breaking anything
        for (i32 x = x_start; x <= x_end; x++) {
            for (i32 z = z_start; z <= z_end; z++) {
                for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                    chunkUpdate(
                        world->chunks[arrayCoord(world, vz, z)]
                                            [arrayCoord(world, vx, x)]
                                            [y],
                        player,
                        NULL
                    );
                }
            }
        }
        DEBUG_LOG("[WORLD] End world update (no breaking state)\n");
        return;
    }
    u8 coords_check = 0b000; // XYZ
    #define updateCoordBit(index, axis) ({ \
        if (axis == chunk_position.v##axis) { \
            coords_check |= 1 << (index); \
        } else { \
            coords_check &= ~(1 << index); \
        } \
    })
    for (i32 x = x_start; x <= x_end; x++) {
        updateCoordBit(2, x);
        for (i32 z = z_start; z <= z_end; z++) {
            updateCoordBit(0, z);
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                updateCoordBit(1, y);
                chunkUpdate(
                    world->chunks[arrayCoord(world, vz, z)]
                                 [arrayCoord(world, vx, x)]
                                 [y],
                    player,
                    coords_check == 0b111 ? breaking_state : NULL
                );
            }
        }
    }
    #undef updateCoordBit
    DEBUG_LOG("[WORLD] End world update (with breaking state)\n");
}

INLINE Chunk* worldGetChunkFromChunkBlock(const World* world, const ChunkBlockPosition* position) {
    // World is void below 0 on y-axis and nothing above height limit
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return NULL;
    }
    const i32 neg_x_limit = world->centre_next.vx - LOADED_CHUNKS_RADIUS;
    if (position->chunk.vx < neg_x_limit) {
        return NULL;
    }
    const i32 pos_x_limit = world->centre_next.vx + LOADED_CHUNKS_RADIUS;
    if (position->chunk.vx > pos_x_limit) {
        return NULL;
    }
    const i32 neg_z_limit = world->centre_next.vz - LOADED_CHUNKS_RADIUS;
    if (position->chunk.vz < neg_z_limit) {
        return NULL;
    }
    const i32 pos_z_limit = world->centre_next.vz + LOADED_CHUNKS_RADIUS;
    if (position->chunk.vz > pos_z_limit) {
        return NULL;
    }
    return world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                        [arrayCoord(world, vx, position->chunk.vx)]
                        [position->chunk.vy];
}

INLINE Chunk* worldGetChunk(const World* world, const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return NULL;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldGetChunkFromChunkBlock(world, &chunk_block_position);
}

IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position) {
    const Chunk* chunk = worldGetChunkFromChunkBlock(world, position);
    if (chunk == NULL) {
        return NULL;
    }
    return chunkGetBlockVec(chunk, &position->block);
}

IBlock* worldGetBlock(const World* world, const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return NULL;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldGetChunkBlock(world, &chunk_block_position);
}

bool worldModifyVoxelChunkBlock(const World* world,
                                const ChunkBlockPosition* position,
                                IBlock* block,
                                const bool drop_item,
                                IItem** item_result) {
    // World is void below 0 on y-axis and nothing above height limit
    if (position->chunk.vy < 0
        || (position->chunk.vy == 0 && position->block.vy < 0)
        || (position->chunk.vy == WORLD_CHUNKS_HEIGHT - 1 && position->block.vy >= CHUNK_SIZE)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return false;
    }
    Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                [arrayCoord(world, vx, position->chunk.vx)]
                                [position->chunk.vy];
    if (chunk == NULL) {
        return false;
    }
    return chunkModifyVoxel(
        chunk,
        &position->block,
        block,
        drop_item,
        item_result
    );
}

IBlock* worldModifyVoxelChunkBlockConstructed(const World* world,
                                           const ChunkBlockPosition* position,
                                           const BlockConstructor block_constructor,
                                           IItem* from_item,
                                           const bool drop_item,
                                           IItem** item_result) {
    // World is void below 0 on y-axis and nothing above height limit
    if (position->chunk.vy < 0
        || (position->chunk.vy == 0 && position->block.vy < 0)
        || (position->chunk.vy == WORLD_CHUNKS_HEIGHT - 1 && position->block.vy >= CHUNK_SIZE)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return NULL;
    }
    Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                [arrayCoord(world, vx, position->chunk.vx)]
                                [position->chunk.vy];
    if (chunk == NULL) {
        return NULL;
    }
    return chunkModifyVoxelConstructed(
        chunk,
        &position->block,
        block_constructor,
        from_item,
        drop_item,
        item_result
    );
}

bool worldModifyVoxel(const World* world,
                      const VECTOR* position,
                      IBlock* block,
                      const bool drop_item,
                      IItem** item_result) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return false;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldModifyVoxelChunkBlock(
        world,
        &chunk_block_position,
        block,
        drop_item,
        item_result
    );
}

IBlock* worldModifyVoxelConstructed(const World* world,
                                 const VECTOR* position,
                                 const BlockConstructor block_constructor,
                                 IItem* from_item,
                                 bool drop_item,
                                 IItem** item_result) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return NULL;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldModifyVoxelChunkBlockConstructed(
        world,
        &chunk_block_position,
        block_constructor,
        from_item,
        drop_item,
        item_result
    );
}

LightLevel worldGetLightValueChunkBlock(const World* world,
                                        const ChunkBlockPosition* position) {
    // World is void below 0 on y-axis and nothing above height limit
    if (position->chunk.vy < 0
        || (position->chunk.vy == 0 && position->block.vy < 0)
        || (position->chunk.vy == WORLD_CHUNKS_HEIGHT - 1 && position->block.vy >= CHUNK_SIZE)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return createLightLevel(0, world->internal_light_level);
    }
    const Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                [arrayCoord(world, vx, position->chunk.vx)]
                                [position->chunk.vy];
    if (chunk == NULL) {
        return createLightLevel(0, world->internal_light_level);
    }
    return chunkGetLightValue(
        chunk,
        &position->block
    );
}

LightLevel worldGetLightValue(const World* world,
                              const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return createLightLevel(0, world->internal_light_level);
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldGetLightValueChunkBlock(
        world,
        &chunk_block_position
    );

}

LightLevel worldGetLightTypeChunkBlock(const World* world,
                                       const ChunkBlockPosition* position,
                                       const LightType light_type) {
    // World is void below 0 on y-axis and nothing above height limit
    if (position->chunk.vy < 0
        || (position->chunk.vy == 0 && position->block.vy < 0)
        || (position->chunk.vy == WORLD_CHUNKS_HEIGHT - 1 && position->block.vy >= CHUNK_SIZE)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return createLightLevel(0, world->internal_light_level);
    }
    const Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                      [arrayCoord(world, vx, position->chunk.vx)]
                                      [position->chunk.vy];
    if (chunk == NULL) {
        return createLightLevel(0, world->internal_light_level);
    }
    return chunkGetLightType(
        chunk,
        &position->block,
        light_type
    );
}

LightLevel worldGetLightType(const World* world,
                             const VECTOR* position,
                             const LightType light_type) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return createLightLevel(0, world->internal_light_level);
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldGetLightTypeChunkBlock(
        world,
        &chunk_block_position,
        light_type
    );
}

void worldSetLightValueChunkBlock(const World* world,
                                  const ChunkBlockPosition* position,
                                  const LightLevel light_value,
                                  const LightType light_type) {
    // World is void below 0 on y-axis and nothing above height limit
    if (position->chunk.vy < 0 || (position->chunk.vy == 0 && position->block.vy < 0)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return;
    }
    Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                [arrayCoord(world, vx, position->chunk.vx)]
                                [position->chunk.vy];
    if (chunk == NULL) {
        return;
    }
    chunkSetLightValue(
        chunk,
        &position->block,
        light_value,
        light_type
    );
}

void worldSetLightValue(const World* world,
                        const VECTOR* position,
                        const LightLevel light_value,
                        const LightType light_type) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    worldSetLightValueChunkBlock(
        world,
        &chunk_block_position,
        light_value,
        light_type
    );
}

void worldRemoveLightType(const World* world,
                          const VECTOR* position,
                          const LightType light_type) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        return;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    worldRemoveLightTypeChunkBlock(
        world,
        &chunk_block_position,
        light_type
    );
}

void worldRemoveLightTypeChunkBlock(const World* world,
                                    const ChunkBlockPosition* position,
                                    const LightType light_type) {

    // World is void below 0 on y-axis and nothing above height limit
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return;
    }
    Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                [arrayCoord(world, vx, position->chunk.vx)]
                                [position->chunk.vy];
    assert(chunk != NULL);
    chunkRemoveLightValue(
        chunk,
        &position->block,
        light_type
    );
}

INLINE LightLevel worldGetInternalLightLevel(const World* world) {
    return world->internal_light_level;
}

INLINE ChunkHeightmap* worldGetChunkHeightmap(World* world, const VECTOR* position) {
    return &world->heightmap[arrayCoord(world, vz, position->vz)]
                            [arrayCoord(world, vx, position->vx)];
}

INLINE Heightmap* worldGetHeightmap(World* world) {
    return &world->heightmap;
}

#define capYPosToWorldHeightSpan(y) max( \
    0, \
    min( \
        WORLD_HEIGHT - 1, \
        y \
    ) \
)

void worldDropItemStack(World* world,
                          IItem* iitem,
                          const u8 count) {
    if (iitem == NULL) {
        // Nothing to drop
        return;
    }
    VECTOR player_block_pos = vec3_const_div(
        vec3_i32(
            player->entity.physics_object.position.vx,
            -player->entity.physics_object.position.vx,
            player->entity.physics_object.position.vz
        ),
        ONE_BLOCK
    );
    player_block_pos.vy = capYPosToWorldHeightSpan(player_block_pos.vy);
#undef capYPosToWorldHeightSpan
    Chunk* chunk = worldGetChunk(world, &player_block_pos);
    assert(chunk != NULL);
    Item* item = VCAST_PTR(Item*, iitem);
    IItem* droppable_iitem = iitem;
    // 0 count implies drop all in this handler
    if (count != 0 && count < item->stack_size) {
        droppable_iitem = itemGetConstructor(item->id)(item->metadata_id);
        assert(droppable_iitem != NULL);
        item = VCAST_PTR(Item*, droppable_iitem);
        item->stack_size = count;
    }
    // Force transition
    item->in_world = false;
    itemSetWorldState(item, true);
    VECTOR velocity = vec3_i32(
        player->entity.physics_object.rotation.pitch,
        player->entity.physics_object.rotation.yaw,
        0
    );
    velocity = rotationToDirection(&velocity);
    velocity = vec3_const_mul(
        velocity,
        4096
    );
    item->world_entity->physics_object.velocity = vec3_i32_all(0);
    // TODO: Add a timestamp marker into the item fields
    //       as the minimum time that it should be considered
    //       when testing for items to pick up around the
    //       player (in chunk.c)
    item->world_entity->physics_object.position = vec3_add(
        player->entity.physics_object.position,
        velocity
    );
    VCALL_SUPER(*droppable_iitem, Renderable, applyWorldRenderAttributes);
    cvector_push_back(chunk->dropped_items, droppable_iitem);
}
