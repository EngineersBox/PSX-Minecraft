// #define DEBUG_LOG_DISABLE 1
#include "world.h"

#include <assert.h>
#include <psxapi.h>
#include <psxgte.h>
#include <inline_c.h>
#include <psxgpu.h>
#include <stdint.h>

#include "../../util/preprocessor.h"
#include "../../util/interface99_extensions.h"
#include "../../util/memory.h"
#include "../../structure/primitive/clip.h"
#include "../../render/duration_tree.h"
#include "../../render/font.h"
#include "../../math/math_utils.h"
#include "../../math/taxicab.h"
#include "../../math/vector.h"
#include "../../ui/progress_bar.h"
#include "../../ui/background.h"
#include "../../logging/logging.h"
#include "../items/items.h"
#include "chunk/chunk.h"
#include "chunk/chunk_defines.h"
#include "chunk/chunk_mesh.h"
#include "chunk/chunk_structure.h"
#include "chunk/chunk_visibility.h"
#include "chunk/heightmap.h"
#include "generation/chunk_provider.h"
#include "position.h"
#include "world_defines.h"

World* world = NULL;

const LightUpdateLimits world_chunk_init_limits = (LightUpdateLimits) {
    .add_block = 0,
    .add_sky = 0,
    .remove_block = 0,
    .remove_sky = 0
};

typedef struct ChunkRenderState {
    ChunkVisibility visibility;
    bool visited: 1;
    u8 _pad: 7;
} ChunkRenderState;

typedef struct ChunkVisit {
    VECTOR position;
    FaceDirection visited_from: FACE_DIRECTION_COUNT_BITS;
    u8 traversal_depth: 5;
} ChunkVisit;
static cvector(ChunkVisit) render_queue = NULL;

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

INLINE World* worldNew() {
    World* world = malloc(sizeof(World));
    assert(world != NULL);
    zeroed(world);
    return world;
}

static void displayProgress(RenderContext* ctx,
                            ProgressBar* progress_bar,
                            const i32 x,
                            const i32 y,
                            const i32 z,
                            const char* msg) {
    // FIXME: Text is not showing over background for some reason
    fontPrintCentreOffset(
        ctx,
        CENTRE_X,
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 3),
        0,
        0,
        "Loading World"
    );
    fontPrintCentreOffset(
        ctx,
        CENTRE_X,
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 2) - 1,
        10,
        0,
        "Chunk [%d,%d,%d]",
        x, y, z
    );
    fontPrintCentreOffset(
        ctx,
        CENTRE_X,
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 1) - 1,
        10,
        0,
        msg,
        x, y, z
    );
    u32* background_ot_object = allocateOrderingTable(ctx, 0);
    progress_bar->value++;
    progressBarRender(progress_bar, 0, ctx);
    backgroundDraw(
        ctx,
        background_ot_object,
        2 * BLOCK_TEXTURE_SIZE,
        0 * BLOCK_TEXTURE_SIZE
    );
    swapBuffers(ctx);
}

// World should be loaded before invoking this method
void worldInit(World* world, RenderContext* ctx) {
    world->internal_light_level = createLightLevel(0, 15);
    world->time_ticks = WORLD_TIME_DAWN;
    world->day_count = 0;
    world->celestial_angle = 0;
    world->centre = vec3_i32(0);
    world->centre_next = vec3_i32(0);
    world->head.vx = 0;
    world->head.vz = 0;
    world->weather = (Weather) {
        .rain_strength = 0,
        .storm_strength = 0,
        .rain_time_ticks = 0,
        .storm_time_ticks = 0,
        .raining = false,
        .storming = false,
    };
    cvector_init(render_queue, 0, NULL);
    // FIXME: Seems like the PSn00bSDK/libpsn00b/libc/memset.s
    //        implementation reads uninitialised memory because
    //        of it's use of the swr instruction variant with no
    //        constant source or address values:
    //        swr   $a1, 0($a0)
    //        which PCSX-Redux implements this by first reading
    //        from the aligned address $0, mutate the 1-4 bytes in
    //        the low (right) end of the value, then write it back.
    //        As such it invokes call(read32Wrapper) which ends up
    //        hitting PCSX::Memory::read32, then msanGetStatus and
    //        thus the error message + breakpoint.
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
    #define WORLD_LOADING_STAGES_COUNT 5
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
        .maximum = ((x_end + 1) - x_start)
            * ((z_end + 1) - z_start)
            * WORLD_CHUNKS_HEIGHT
            * WORLD_LOADING_STAGES_COUNT
    };
    DEBUG_LOG("Loading chunks\n");
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
    DEBUG_LOG("Generating Lightmaps\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("Generating Lightmap\n", x, 0, z);
                const u16 array_x = arrayCoord(world, vx, x);
                const u16 array_z = arrayCoord(world, vz, z);
                Chunk* chunk = world->chunks[array_z][array_x][y];
                chunkGenerateLightmap(chunk, &gen_ctx[array_z][array_z][y]);
                displayProgress(ctx, &bar, x, y, z, "Generating Lightmap");
            }
        }
    }
    DEBUG_LOG("Propagating Light\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("Propagating Light\n", x, 0, z);
                const u16 array_x = arrayCoord(world, vx, x);
                const u16 array_z = arrayCoord(world, vz, z);
                Chunk* chunk = world->chunks[array_z][array_x][y];
                chunkPropagateLightmap(chunk, &gen_ctx[array_z][array_x][y]);
                displayProgress(ctx, &bar, x, y, z, "Propagating Light");
            }
        }
    }
    DEBUG_LOG("Processing Light Updates\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("Processing Light Updates\n", x, 0, z);
           Chunk* chunk = world->chunks[arrayCoord(world, vz, z)]
                                       [arrayCoord(world, vx, x)]
                                       [y];
                chunkUpdateLight(chunk, world_chunk_init_limits);
                displayProgress(ctx, &bar, x, y, z, "Processing Light Updates");
            }
        }
    }
    DEBUG_LOG("Building chunk meshes\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("Generating mesh\n", x, 0, z);
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
#undef WORLD_LOADING_STAGES_COUNT
#undef displayProgress
    DEBUG_LOG("Finished loading\n");
    // abort();
}

void worldDestroy(World* world) {
    cvector_free(render_queue);
    render_queue = NULL;
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

#define SUN_DISTANCE 400
#define SUN_SIZE 200

static const VECTOR SUN_MOON_VERTICES[4] = {
    [0]=vec3_i32( SUN_SIZE, SUN_SIZE, 0),
    [1]=vec3_i32( SUN_SIZE, -SUN_SIZE, 0),
    [2]=vec3_i32(-SUN_SIZE, SUN_SIZE, 0),
    [3]=vec3_i32(-SUN_SIZE, -SUN_SIZE, 0)
};

static const DVECTOR ANGLE_POS[90] = {
    [0]=(DVECTOR){ .vx=ONE, .vy=0 },
    [1]=(DVECTOR){ .vx=4095, .vy=71 },
    [2]=(DVECTOR){ .vx=4093, .vy=142 },
    [3]=(DVECTOR){ .vx=4090, .vy=214 },
    [4]=(DVECTOR){ .vx=4086, .vy=285 },
    [5]=(DVECTOR){ .vx=4080, .vy=356 },
    [6]=(DVECTOR){ .vx=4073, .vy=428 },
    [7]=(DVECTOR){ .vx=4065, .vy=499 },
    [8]=(DVECTOR){ .vx=4056, .vy=570 },
    [9]=(DVECTOR){ .vx=4045, .vy=640 },
    [10]=(DVECTOR){ .vx=4033, .vy=711 },
    [11]=(DVECTOR){ .vx=4020, .vy=781 },
    [12]=(DVECTOR){ .vx=4006, .vy=851 },
    [13]=(DVECTOR){ .vx=3991, .vy=921 },
    [14]=(DVECTOR){ .vx=3974, .vy=990 },
    [15]=(DVECTOR){ .vx=3956, .vy=1060 },
    [16]=(DVECTOR){ .vx=3937, .vy=1129 },
    [17]=(DVECTOR){ .vx=3917, .vy=1197 },
    [18]=(DVECTOR){ .vx=3895, .vy=1265 },
    [19]=(DVECTOR){ .vx=3872, .vy=1333 },
    [20]=(DVECTOR){ .vx=3848, .vy=1400 },
    [21]=(DVECTOR){ .vx=3823, .vy=1467 },
    [22]=(DVECTOR){ .vx=3797, .vy=1534 },
    [23]=(DVECTOR){ .vx=3770, .vy=1600 },
    [24]=(DVECTOR){ .vx=3741, .vy=1665 },
    [25]=(DVECTOR){ .vx=3712, .vy=1731 },
    [26]=(DVECTOR){ .vx=3681, .vy=1795 },
    [27]=(DVECTOR){ .vx=3649, .vy=1859 },
    [28]=(DVECTOR){ .vx=3616, .vy=1922 },
    [29]=(DVECTOR){ .vx=3582, .vy=1985 },
    [30]=(DVECTOR){ .vx=3547, .vy=2047 },
    [31]=(DVECTOR){ .vx=3510, .vy=2109 },
    [32]=(DVECTOR){ .vx=3473, .vy=2170 },
    [33]=(DVECTOR){ .vx=3435, .vy=2230 },
    [34]=(DVECTOR){ .vx=3395, .vy=2290 },
    [35]=(DVECTOR){ .vx=3355, .vy=2349 },
    [36]=(DVECTOR){ .vx=3313, .vy=2407 },
    [37]=(DVECTOR){ .vx=3271, .vy=2465 },
    [38]=(DVECTOR){ .vx=3227, .vy=2521 },
    [39]=(DVECTOR){ .vx=3183, .vy=2577 },
    [40]=(DVECTOR){ .vx=3137, .vy=2632 },
    [41]=(DVECTOR){ .vx=3091, .vy=2687 },
    [42]=(DVECTOR){ .vx=3043, .vy=2740 },
    [43]=(DVECTOR){ .vx=2995, .vy=2793 },
    [44]=(DVECTOR){ .vx=2946, .vy=2845 },
    [45]=(DVECTOR){ .vx=2896, .vy=2896 },
    [46]=(DVECTOR){ .vx=2845, .vy=2946 },
    [47]=(DVECTOR){ .vx=2793, .vy=2995 },
    [48]=(DVECTOR){ .vx=2740, .vy=3043 },
    [49]=(DVECTOR){ .vx=2687, .vy=3091 },
    [50]=(DVECTOR){ .vx=2632, .vy=3137 },
    [51]=(DVECTOR){ .vx=2577, .vy=3183 },
    [52]=(DVECTOR){ .vx=2521, .vy=3227 },
    [53]=(DVECTOR){ .vx=2465, .vy=3271 },
    [54]=(DVECTOR){ .vx=2407, .vy=3313 },
    [55]=(DVECTOR){ .vx=2349, .vy=3355 },
    [56]=(DVECTOR){ .vx=2290, .vy=3395 },
    [57]=(DVECTOR){ .vx=2230, .vy=3435 },
    [58]=(DVECTOR){ .vx=2170, .vy=3473 },
    [59]=(DVECTOR){ .vx=2109, .vy=3510 },
    [60]=(DVECTOR){ .vx=2048, .vy=3547 },
    [61]=(DVECTOR){ .vx=1985, .vy=3582 },
    [62]=(DVECTOR){ .vx=1922, .vy=3616 },
    [63]=(DVECTOR){ .vx=1859, .vy=3649 },
    [64]=(DVECTOR){ .vx=1795, .vy=3681 },
    [65]=(DVECTOR){ .vx=1731, .vy=3712 },
    [66]=(DVECTOR){ .vx=1665, .vy=3741 },
    [67]=(DVECTOR){ .vx=1600, .vy=3770 },
    [68]=(DVECTOR){ .vx=1534, .vy=3797 },
    [69]=(DVECTOR){ .vx=1467, .vy=3823 },
    [70]=(DVECTOR){ .vx=1400, .vy=3848 },
    [71]=(DVECTOR){ .vx=1333, .vy=3872 },
    [72]=(DVECTOR){ .vx=1265, .vy=3895 },
    [73]=(DVECTOR){ .vx=1197, .vy=3917 },
    [74]=(DVECTOR){ .vx=1129, .vy=3937 },
    [75]=(DVECTOR){ .vx=1060, .vy=3956 },
    [76]=(DVECTOR){ .vx=990, .vy=3974 },
    [77]=(DVECTOR){ .vx=921, .vy=3991 },
    [78]=(DVECTOR){ .vx=851, .vy=4006 },
    [79]=(DVECTOR){ .vx=781, .vy=4020 },
    [80]=(DVECTOR){ .vx=711, .vy=4033 },
    [81]=(DVECTOR){ .vx=640, .vy=4045 },
    [82]=(DVECTOR){ .vx=570, .vy=4056 },
    [83]=(DVECTOR){ .vx=499, .vy=4065 },
    [84]=(DVECTOR){ .vx=428, .vy=4073 },
    [85]=(DVECTOR){ .vx=356, .vy=4080 },
    [86]=(DVECTOR){ .vx=285, .vy=4086 },
    [87]=(DVECTOR){ .vx=214, .vy=4090 },
    [88]=(DVECTOR){ .vx=142, .vy=4093 },
    [89]=(DVECTOR){ .vx=71, .vy=4095 }
};

void worldRenderSkybox(const World* world,
                       RenderContext* ctx,
                       Transforms* transforms) {
    const fixedi32 angle = fixedFixedDiv(world->time_ticks + 1, WORLD_TIME_CYCLE);
    SVECTOR rotation = vec3_i16(
        positiveModulo(-angle - FIXED_1_4, ONE),
        0,
        0
    );
    i32 sun_z = 0;
    i32 sun_y = 0;
    const i32 deg = unitRangeToDeg(angle);
    // DEBUG_LOG("Angle: %d Deg: %d\n", angle, deg);
    const DVECTOR* pos = NULL;
    switch (deg) {
        case 0 ... 89:
            pos = &ANGLE_POS[deg];
            sun_z = fixedMul(pos->vx, -SUN_DISTANCE);
            sun_y = fixedMul(pos->vy, -SUN_DISTANCE);
            break;
        case 90 ... 179:
            pos = &ANGLE_POS[89 - (deg % 90)];
            sun_z = fixedMul(pos->vx, SUN_DISTANCE);
            sun_y = fixedMul(pos->vy, -SUN_DISTANCE);
            break;
        case 180 ... 269:
            pos = &ANGLE_POS[deg % 90];
            sun_z = fixedMul(pos->vx, SUN_DISTANCE);
            sun_y = fixedMul(pos->vy, SUN_DISTANCE);
            break;
        case 270 ... 360:
            pos = &ANGLE_POS[89 - (deg % 90)];
            sun_z = fixedMul(pos->vx, -SUN_DISTANCE);
            sun_y = fixedMul(pos->vy, SUN_DISTANCE);
            break;
    }
    VECTOR position = vec3_i32(
        0,
        sun_y,
        sun_z
    );
    // DEBUG_LOG("Pos: " VEC_PATTERN "\n", VEC_LAYOUT(position));
    // Object and light matrix for object
    MATRIX omtx = {0};
    // Set object rotation and position
    RotMatrix(&rotation, &omtx);
    TransMatrix(&omtx, &position);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&transforms->skybox_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    const u32* ot_object = allocateOrderingTable(ctx, ORDERING_TABLE_LENGTH - 1);
    setPolyFT4(pol4);
    setRGB0(pol4, 0x80, 0x80, 0x80);
    gte_ldv3(
        &SUN_MOON_VERTICES[0],
        &SUN_MOON_VERTICES[1],
        &SUN_MOON_VERTICES[2]
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
    // Set the projected vertices to the primitive
    gte_stsxy0(&pol4->x0);
    gte_stsxy1(&pol4->x1);
    gte_stsxy2(&pol4->x2);
    // Compute the last vertex and set the result
    gte_ldv0(&SUN_MOON_VERTICES[3]);
    gte_rtps();
    gte_stsxy(&pol4->x3);
    // Test if quad is off-screen, discard if so
    if (quadClip(
        &ctx->screen_clip,
        (DVECTOR*) &pol4->x0,
        (DVECTOR*) &pol4->x1,
        (DVECTOR*) &pol4->x2,
        (DVECTOR*) &pol4->x3)) {
        goto render_moon;
    }
    setUVWH(pol4, 0, 160, 32, 32);
    const Texture* sun_texture = &textures[ASSET_TEXTURE__STATIC__SUN];
    pol4->tpage = sun_texture->tpage;
    pol4->clut = sun_texture->clut;
    addPrim(ot_object, pol4);
    renderClearConstraintsEntry(ctx, (u32*) ot_object);
    renderCtxUnbindMatrix();
    return;
render_moon:;
    rotation.vx = positiveModulo(rotation.vx + FIXED_1_2, ONE);
    position.vy *= -1;
    position.vz *= -1;
    omtx = (MATRIX) {0};
    // Set object rotation and position
    RotMatrix(&rotation, &omtx);
    TransMatrix(&omtx, &position);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&transforms->skybox_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    gte_ldv3(
        &SUN_MOON_VERTICES[0],
        &SUN_MOON_VERTICES[1],
        &SUN_MOON_VERTICES[2]
    );
    // Rotation, Translation and Perspective Triple
    gte_rtpt();
    gte_nclip();
    gte_stopz(&p);
    // gte_stdp(&dp);
    // Avoid negative depth (behind camera) and zero
    // for constraint clearing primitive in OT
    if (p < 0) {
        goto no_sun_or_moon;
    }
    // Set the projected vertices to the primitive
    gte_stsxy0(&pol4->x0);
    gte_stsxy1(&pol4->x1);
    gte_stsxy2(&pol4->x2);
    // Compute the last vertex and set the result
    gte_ldv0(&SUN_MOON_VERTICES[3]);
    gte_rtps();
    gte_stsxy(&pol4->x3);
    // Test if quad is off-screen, discard if so
    if (quadClip(
        &ctx->screen_clip,
        (DVECTOR*) &pol4->x0,
        (DVECTOR*) &pol4->x1,
        (DVECTOR*) &pol4->x2,
        (DVECTOR*) &pol4->x3)) {
        goto no_sun_or_moon;
    }
    setUVWH(pol4, 32, 160, 32, 32);
    const Texture* moon_texture = &textures[ASSET_TEXTURE__STATIC__MOON];
    pol4->tpage = moon_texture->tpage;
    pol4->clut = moon_texture->clut;
    addPrim(ot_object, pol4);
    renderClearConstraintsEntry(ctx, (u32*) ot_object);
    renderCtxUnbindMatrix();
    return;
no_sun_or_moon:;
    freePrimitive(ctx, sizeof(POLY_FT4));
    renderCtxUnbindMatrix();
}

INLINE bool worldIsOutsideBounds(const World* world, const ChunkBlockPosition* position) {
    // World is void below 0 on y-axis and nothing above height limit
    if (position->chunk.vy < 0 || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return true;
    }
    const i32 neg_x_limit = world->centre_next.vx - LOADED_CHUNKS_RADIUS;
    if (position->chunk.vx < neg_x_limit) {
        return true;
    }
    const i32 pos_x_limit = world->centre_next.vx + LOADED_CHUNKS_RADIUS;
    if (position->chunk.vx > pos_x_limit) {
        return true;
    }
    const i32 neg_z_limit = world->centre_next.vz - LOADED_CHUNKS_RADIUS;
    if (position->chunk.vz < neg_z_limit) {
        return true;
    }
    const i32 pos_z_limit = world->centre_next.vz + LOADED_CHUNKS_RADIUS;
    if (position->chunk.vz > pos_z_limit) {
        return true;
    }
    return false;
}

/**
 * TODO: Explain this with doc:
 *       1. Three verts visible to direction vector quadrant
 *       2. Naievly encoded as array of vertex offsets from chunk base coord
 *       3. Compressed into u8 for each set of offets per-quadrant
 *       4. Compressed into single u32, shifted with quadrant index
 */
static const u32 quadrant_verts = 0b00000000100111110001001110011000;

// static const u8 quadrant_verts[4] = {
//     [0]=0b00011000,
//     [1]=0b00001110,
//     [2]=0b00110001,
//     [3]=0b00100111,
//     // [0]={
//     //     [0]=vec3_i32(0,0,0),
//     //     [1]=vec3_i32(1,0,0),
//     //     [2]=vec3_i32(0,1,0),
//     // },
//     // [1]={
//     //     [0]=vec3_i32(1,0,0),
//     //     [1]=vec3_i32(1,1,0),
//     //     [2]=vec3_i32(0,0,0),
//     // },
//     // [2]={
//     //     [0]=vec3_i32(0,1,0),
//     //     [1]=vec3_i32(0,0,0),
//     //     [2]=vec3_i32(1,1,0),
//     // },
//     // [3]={
//     //     [0]=vec3_i32(1,1,0),
//     //     [1]=vec3_i32(0,1,0),
//     //     [2]=vec3_i32(1,0,0),
//     // },
// };

static bool verticiesVisible(const i32 z,
                             const i32 q,
                             const i32 chunk_rel_z,
                             const i32 chunk_rel_q,
                             const i32 angle_ref,
                             const i32 angle) {
    // const size_t quadrant = (y > 0 ? 0 : 2) + (x > 0 ? 0 : 1);
    const size_t quadrant = ((z <= 0) * 2) + (q <= 0);
    u8 quadrant_vert = quadrant_verts >> (quadrant * 6);
    #pragma GCC unroll(3)
    for (size_t i = 0; i < 3; i++) {
        const i32 qv_z = quadrant_vert & 0b1;
        quadrant_vert >>= 1;
        const i32 qv_q = quadrant_vert & 0b1;
        quadrant_vert >>= 1;
        const TRad query = tcabAngle(
            chunk_rel_z + (qv_z * (CHUNK_SIZE << FIXED_POINT_SHIFT)),
            chunk_rel_q + (qv_q * (CHUNK_SIZE << FIXED_POINT_SHIFT))
        );
        if (tcabAngleInRange(angle_ref, angle, query)) {
            return true;
        }
    }
    return false;
}

DEFN_DURATION_COMPONENT(world_render);

void worldRender(const World* world,
                 const Player* player,
                 RenderContext* ctx,
                 Transforms* transforms) {
    DEBUG_LOG("==== START WORLD RENDER ====\n");
    durationComponentInitOnce(world_render, "worldRender");
    durationComponentStart(&world_render_duration);
    const VECTOR player_world_pos = vec3_const_div(
        vec3_i32(
            fixedFloor(player->camera->position.vx, ONE_BLOCK),
            fixedFloor(-player->camera->position.vy, ONE_BLOCK),
            fixedFloor(player->camera->position.vz, ONE_BLOCK)
        ),
        ONE_BLOCK
    );
    worldRenderSkybox(world, ctx, transforms);
    const ChunkBlockPosition player_pos = worldToChunkBlockPosition(
        &player_world_pos,
        CHUNK_SIZE
    );
    // DEBUG_LOG("Player chunk pos: " VEC_PATTERN "\n", VEC_LAYOUT(player_pos.chunk));
    // Pich = up and down
    const TRad playerTRadPitch = player->camera->rotation.vx >> 10;
    // Yaw = left and right
    const TRad playerTRadYaw = player->camera->rotation.vy >> 10;
    const FaceDirection player_camera_direction = faceDirectionClosestNormal(player->camera->direction);
    cvector_push_back(
        render_queue,
        ((ChunkVisit) {
            .position = player_pos.chunk,
            .visited_from = faceDirectionOpposing(player_camera_direction),
            .traversal_depth = 0,
        })
    );
    ChunkRenderState chunk_render_states[AXIS_CHUNKS * WORLD_CHUNKS_HEIGHT] = {0};
    #define getChunkRenderState(x, y, z) (chunk_render_states[((y) * AXIS_LOADED_CHUNKS) + (z)])
    // size_t render_count = 0;
    while (cvector_size(render_queue) > 0) {
        const ChunkVisit visit = render_queue[cvector_size(render_queue) - 1];
        cvector_pop_back(render_queue);
        // DEBUG_LOG(
        //     "Visit chunk " VEC_PATTERN " @ " VEC_PATTERN "\n", 
        //     VEC_LAYOUT(visit.position),
        //     arrayCoord(world, vx, visit.position.vx),
        //     visit.position.vy,
        //     arrayCoord(world, vz, visit.position.vz)
        // );
        ChunkBlockPosition chunk_pos = (ChunkBlockPosition) {
            .chunk = visit.position,
            .block = vec3_i32(0)
        };
        Chunk* chunk = worldGetChunkFromChunkBlock(world, &chunk_pos);
        // NOTE: If chunk is NULL, then always traverse to next chunks, ignoring
        //       visibility, since it's always visible.
        ChunkRenderState allVisCrs = (ChunkRenderState) {
            .visibility = UINT16_MAX,
            .visited = true
        };
        ChunkRenderState* chunk_render_state = chunk == NULL ? &allVisCrs : &getChunkRenderState(
            chunk_pos.chunk.vx,
            chunk_pos.chunk.vy,
            chunk_pos.chunk.vz
        );
        if (chunk_render_state->visibility == 0 && !chunk_render_state->visited) {
            chunk_render_state->visibility = chunk->visibility;
            chunk_render_state->visited = true;
            chunkRender(
                chunk,
                vec3_equal(chunk_pos.chunk, visit.position),
                true,
                ctx,
                transforms
            );
        }
            // render_count++;
        // DEBUG_LOG("Chunk vis: " INT16_BIN_PATTERN "\n", INT16_BIN_LAYOUT(chunk_render_state->visibility));
        if (chunk_render_state->visibility == 0) {
            // Can't see anything, don't bother
            // DEBUG_LOG("No visibility\n");
            continue;
        } else if (visit.traversal_depth > WORLD_RENDER_DISTANCE) {
            // DEBUG_LOG("Exceeded max render distance\n");
            continue;
        }
        for (FaceDirection face_dir = FACE_DIR_DOWN; face_dir <= FACE_DIR_FRONT; face_dir++) {
            if (face_dir == visit.visited_from) {
                // DEBUG_LOG("Same direction as visited from %d == %d\n", face_dir, visit.visited_from);
                chunkVisibilityClearBit(&chunk_render_state->visibility, face_dir, visit.visited_from);
                continue;
            } else if (visit.traversal_depth != 0 && chunkVisibilityGetBit(
                    chunk_render_state->visibility,
                    face_dir,
                    visit.visited_from
                ) == 0) {
                // Cannot exit chunk in this direction from the entered face
                // DEBUG_LOG("No visibility from face %d to %d\n", visit.visited_from, face_dir);
                continue;
            }
            chunkVisibilityClearBit(&chunk_render_state->visibility, face_dir, visit.visited_from);
            // DEBUG_LOG("Visible from face %d to %d\n", visit.visited_from, face_dir);
            const VECTOR next_chunk = vec3_add(visit.position, FACE_DIRECTION_NORMALS[face_dir]);
            // DEBUG_LOG("Next chunk: " VEC_PATTERN "\n", VEC_LAYOUT(next_chunk));
            const ChunkBlockPosition next_cb_pos = (ChunkBlockPosition) {
                .chunk = next_chunk,
                .block = vec3_i32(0)
            };
                // Check if traversal direction is back towards camera. Skip if so.
            if (dot_i32(next_chunk, player_pos.chunk) < 0) {
                continue;
            }
            // If current position is within world bounds and next
            // position is out of bounds, ignore the next position
            // as it will just lead to pointless iterations.
            if (chunk != NULL && worldIsOutsideBounds(world, &next_cb_pos)) {
                // DEBUG_LOG("Outside world\n");
                continue;
            }
            // BUG: Need to prevent traversal outside the world from looping
            //      indefinitely (i.e. when flying outside of world chunks)
            const VECTOR chunk_relative_pos = vec3_sub(next_chunk, player_pos.chunk);
            if (vec3_equal(chunk_relative_pos, VEC3_I32_ZERO)) {
                continue;
            }
            // DEBUG_LOG("Chunk relative pos: " VEC_PATTERN "\n", VEC_LAYOUT(chunk_relative_pos));
            const VECTOR chunk_relative_pos_blocks = vec3_const_mul(chunk_relative_pos, CHUNK_SIZE << FIXED_POINT_SHIFT);
            // DEBUG_LOG("Chunk relative pos blocks: " VEC_PATTERN "\n", VEC_LAYOUT(chunk_relative_pos_blocks));
            const VECTOR chunk_relative_centre_blocks = vec3_const_add(chunk_relative_pos_blocks, ((CHUNK_SIZE >> 1) << FIXED_POINT_SHIFT));
            // DEBUG_LOG("Chunk relative centre blocks: " VEC_PATTERN "\n", VEC_LAYOUT(chunk_relative_centre_blocks));
            /* Chunk render logic:
             * 1. Compute relative chunk centre point
             * 2. Test if chunk centre is in frustum (dual angle check), skip if not visible
             * 3. Otherwise, get quadrant closest vertices of chunk
             * 4. Test if verticies are in frustum (dual angle check), skip if not visible
             * 5. Render chunk
             *
             * NOTE: Test for chunk centre is necessary in the case that nearest vertices
             *       to direction ray are outside of frustum, but chunk is still visible.
             */
            // 1. Centre check
            const TRad chunkTRadPitch = tcabAngle(chunk_relative_centre_blocks.vz, chunk_relative_centre_blocks.vy);
            const bool pitch_in_range = tcabAngleInRange(playerTRadPitch, FOV_Y_HALF_TRAD, chunkTRadPitch);
            // DEBUG_LOG("Chunk pitch t-rad: %d Player pitch t-rad: %d In range: %d\n", chunkTRadPitch, playerTRadPitch, pitch_in_range);
            if (!pitch_in_range) goto check_chunk_vertex_visibility;
            const TRad chunkTRadYaw = tcabAngle(chunk_relative_centre_blocks.vz, chunk_relative_centre_blocks.vx);
            const bool yaw_in_range = tcabAngleInRange(playerTRadYaw, FOV_X_HALF_TRAD, chunkTRadYaw);
            // DEBUG_LOG("Chunk yaw t-rad: %d Player yaw t-rad: %d In range: %d\n", chunkTRadYaw, playerTRadYaw, yaw_in_range);
            if (yaw_in_range) {
                goto push_chunk_to_render_queue;
            }
check_chunk_vertex_visibility:;
            // 2, Quadrant verticies check
            if (!verticiesVisible(
                next_cb_pos.chunk.vz,
				next_cb_pos.chunk.vy,
				chunk_relative_pos_blocks.vz,
				chunk_relative_pos_blocks.vy,
				playerTRadPitch,
				FOV_Y_HALF_TRAD
            )) {
                // DEBUG_LOG("Chunk ZY/pitch vertices not visible\n");
                continue;
            } else if (!verticiesVisible(
                next_cb_pos.chunk.vz,
				next_cb_pos.chunk.vx,
				chunk_relative_pos_blocks.vz,
				chunk_relative_pos_blocks.vx,
				playerTRadYaw,
				FOV_X_HALF_TRAD
            )) {
                // DEBUG_LOG("Chunk ZX/yaw vertices not visible\n");
                continue;
            }
push_chunk_to_render_queue:;
            // DEBUG_LOG("In-frustum\n");
            cvector_push_back(
                render_queue,
                ((ChunkVisit) {
                    .position = next_chunk,
                    .visited_from = faceDirectionOpposing(face_dir),
                    .traversal_depth = visit.traversal_depth + 1
                })
            );
        }
    }
    // DEBUG_LOG("Chunks rendered: %d\n", render_count);
    #undef getChunkRenderState
    durationComponentEnd();
    durationTreeRender(
        durationComponentCurrentAtIndex(world_render_duration.index),
        ctx,
        transforms
    );
    DEBUG_LOG("==== END WORLD RENDER ====\n");
    // pcsx_debugbreak();
}

void worldRenderOld(const World* world,
                    const Player* player,
                    RenderContext* ctx,
                    Transforms* transforms) {
    durationComponentInitOnce(world_render, "worldRender");
    durationComponentStart(&world_render_duration);
    const VECTOR player_world_pos = vec3_i32(
        fixedFloor(player->entity.physics_object.position.vx, ONE_BLOCK) / ONE_BLOCK,
        fixedFloor(player->entity.physics_object.position.vy, ONE_BLOCK) / ONE_BLOCK,
        fixedFloor(player->entity.physics_object.position.vz, ONE_BLOCK) / ONE_BLOCK
    );
    worldRenderSkybox(world, ctx, transforms);
    const ChunkBlockPosition cb_pos = worldToChunkBlockPosition(
        &player_world_pos,
        CHUNK_SIZE
    );
    const i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const i32 x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    // TODO: Render current chunk and track how much of the screen has been drawn (somehow?)
    //       if there are still bits that are missing traverse to next chunks in the direction
    //       the player is facing and render them. Stop drawing if screen is full and/or there
    //       are no more loaded chunks to traverse to.
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                Chunk* chunk = world->chunks[arrayCoord(world, vz, z)]
                                 [arrayCoord(world, vx, x)]
                                 [y];
                chunkRender(
                    chunk,
                    cb_pos.chunk.vx == x
                        && cb_pos.chunk.vz == z
                        && cb_pos.chunk.vy == y,
                    false,
                    ctx,
                    transforms
                );
            }
        }
    }
    durationComponentEnd();
    durationTreeRender(
        durationComponentCurrentAtIndex(world_render_duration.index),
        ctx,
        transforms
    );
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
        worldLoadChunksX(world, x_direction, z_direction);
    }
    if (z_direction != 0) {
        worldLoadChunksZ(world, x_direction, z_direction);
    }
    if (x_direction != 0 && z_direction != 0) {
        worldLoadChunksXZ(world, x_direction, z_direction);
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

static fixedi32 calculateCelestialAngle(u16 time_ticks) {
    fixedi32 scaled = fixedFixedDiv(time_ticks + 1, WORLD_TIME_CYCLE) - FIXED_1_4;
    if (scaled < 0) {
        scaled += ONE;
    }
    if (scaled > ONE) {
        scaled -= ONE;
    }
    fixedi32 cached_scaled = scaled;
    scaled = ONE - ((cos5o(fixedMul(scaled, FIXED_PI)) + ONE) >> 1);
    // NOTE: Using (ONE * 1/3) = 1365.3... approximation
    //       to do (scaled - cached_scaled) * 1365 instead
    //       of div by 3. Accuracy isn't that important here
    // scaled = cached_scaled + ((scaled - cached_scaled) / 3);
    scaled = cached_scaled + fixedMul((scaled - cached_scaled), 1365);
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
    const CVECTOR ambient_colour = vec3_rgb(colour);
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
    #undef setLevel
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
    const VECTOR player_world_pos = vec3_const_div(
        vec3_i32(
            fixedFloor(player->entity.physics_object.position.vx, ONE_BLOCK),
            fixedFloor(-player->entity.physics_object.position.vy, ONE_BLOCK),
            fixedFloor(player->entity.physics_object.position.vz, ONE_BLOCK)
        ),
        ONE_BLOCK
    );
    const ChunkBlockPosition player_pos = worldToChunkBlockPosition(
        &player_world_pos,
        CHUNK_SIZE
    );
    if (isPlayerInEdgeChunks(world, &player_pos)) {
        // DEBUG_LOG("Player chunk pos: " VEC_PATTERN "\n", VEC_LAYOUT(player_pos.chunk));
        worldLoadChunks(world, &player_pos.chunk);
        // DEBUG_LOG(
        //     "Head { x: %d, z: %d } Centre { x: %d, z: %d}\n",
        //     world->head.vx, world->head.vz,
        //     world->centre.vx, world->centre.vz
        // );
        // for (i32 z = 0; z < AXIS_CHUNKS; z++) {
        //     for (i32 x = 0; x < AXIS_CHUNKS; x++) {
        //         DEBUG_LOG("%d ", world->chunks[z][x][0] != NULL);
        //     }
        //     DEBUG_LOG("\n");
        // }
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
        // DEBUG_LOG("End world update (no breaking state)\n");
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
    // DEBUG_LOG("End world update (with breaking state)\n");
}

INLINE Chunk* worldGetChunkFromChunkBlock(const World* world, const ChunkBlockPosition* position) {
    if (worldIsOutsideBounds(world, position)) {
        return NULL;
    }
    return world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                        [arrayCoord(world, vx, position->chunk.vx)]
                        [position->chunk.vy];
}

INLINE Chunk* worldGetChunk(const World* world, const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_CHUNKS_HEIGHT) {
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
    // Ensure that item uses correct world rendering attributes
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
    item->world_entity->physics_object.velocity = vec3_i32(0);
    item->world_entity->physics_object.position = vec3_add(
        player->entity.physics_object.position,
        velocity
    );
    VCALL_SUPER(*droppable_iitem, Renderable, applyWorldRenderAttributes);
    cvector_push_back(
        chunk->dropped_items,
        ((DroppedIItem) {
            .iitem = droppable_iitem,
            .lifetime = ITEM_DROPPED_LIFETIME_MS
        })
    );
}
