#include "world.h"

#include <assert.h>
#include <psxapi.h>
#include <psxgte.h>

#include "../../util/interface99_extensions.h"
#include "../render/font.h"
#include "../math/math_utils.h"
#include "../math/vector.h"
#include "../ui/progress_bar.h"
#include "../ui/background.h"
#include "../../logging/logging.h"
#include "chunk/chunk.h"
#include "chunk/chunk_structure.h"

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

// World should be loaded before invoking this method
void worldInit(World* world, RenderContext* ctx) {
    // TODO: Set light level based on time of day
    world->internal_light_level = createLightLevel(0, 15);
    VCALL(world->chunk_provider, init);
    // Clear the chunks first to ensure they are all NULL upon initialisation
    for (i32 x = 0; x < AXIS_CHUNKS; x++) {
        for (i32 z = 0; z < AXIS_CHUNKS; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                world->chunks[z][x][y] = NULL;
            }
        }
    }
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
#define displayProgress(msg) \
    fontPrintCentreOffset( \
        ctx, \
        CENTRE_X, \
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 3), \
        0, \
        "Loading World" \
    ); \
    fontPrintCentreOffset( \
        ctx, \
        CENTRE_X, \
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 2) - 1, \
        10, \
        "Chunk [%d,%d,%d]", \
        x, y, z \
    ); \
    fontPrintCentreOffset( \
        ctx, \
        CENTRE_X, \
        CENTRE_Y - ((FONT_CHARACTER_SPRITE_HEIGHT + 2) * 1) - 1, \
        10, \
        msg, \
        x, y, z \
    ); \
    backgroundDraw(ctx, 2, 2 * BLOCK_TEXTURE_SIZE, 0 * BLOCK_TEXTURE_SIZE); \
    bar.value++; \
    progressBarRender(&bar, 1, ctx); \
    swapBuffers(ctx);
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
                displayProgress("Loading Chunk Data");
            }
        }
    }
    DEBUG_LOG("[WORLD] Generating Lightmaps\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("[CHUNK: %d,%d,%d] Generating Lightmap\n", x, 0, z);
                Chunk* chunk = world->chunks[arrayCoord(world, vz, z)]
                                            [arrayCoord(world, vx, x)]
                                            [y];
                chunkGenerateLightmap(chunk);
                displayProgress("Generating Lightmap");
            }
        }
    }
    DEBUG_LOG("[WORLD] Propagating Light\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                DEBUG_LOG("[CHUNK: %d,%d,%d] Propagating Light\n", x, 0, z);
                Chunk* chunk = world->chunks[arrayCoord(world, vz, z)]
                                            [arrayCoord(world, vx, x)]
                                            [y];
                chunkPropagateLightmap(chunk);
                displayProgress("Propagating Light");
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
                displayProgress("Processing Light Updates");
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
                displayProgress("Building Mesh");
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

void worldRender(const World* world,
                 RenderContext* ctx,
                 Transforms* transforms) {
    // PERF: Revamp with BFS for visible chunks occlusion (use frustum culling too?)
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
                chunkRender(
                    world->chunks[arrayCoord(world, vz, z)]
                                 [arrayCoord(world, vx, x)]
                                 [y],
                    ctx,
                    transforms
                );
            }
        }
    }
    return;
}

// NOTE: Should this just take i32 x,y,z params instead of a
//       a VECTOR struct to avoid creating needless stack objects?
Chunk* worldLoadChunk(World* world, const VECTOR chunk_position) {
    Chunk* chunk = VCALL(world->chunk_provider, provide, chunk_position);
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
            chunkGenerateLightmap(to_mesh[i++]);
        }
    }
    i = 0;
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkPropagateLightmap(to_mesh[i++]);
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
            chunkGenerateLightmap(to_mesh[i++]);
        }
    }
    i = 0;
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            chunkPropagateLightmap(to_mesh[i++]);
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
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk* loaded_chunk = worldLoadChunk(world, vec3_i32(x_coord, y, z_coord));
        world->chunks[arrayCoord(world, vz, z_coord)]
                     [arrayCoord(world, vx, x_coord)]
                     [y] = to_mesh[y] = loaded_chunk;
    }
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        chunkGenerateLightmap(to_mesh[y]);
    }
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        chunkPropagateLightmap(to_mesh[y]);
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

INLINE bool worldWithinLoadRadius(const World* world, const VECTOR* player_chunk_pos) {
    return absv(world->centre.vx - player_chunk_pos->vx) < LOADED_CHUNKS_RADIUS - 1
           && absv(world->centre.vz - player_chunk_pos->vz) < LOADED_CHUNKS_RADIUS - 1;
}

void worldLoadChunks(World* world, const VECTOR* player_chunk_pos) {
    // Check if we need to load
    if (worldWithinLoadRadius(world, player_chunk_pos)) {
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

void worldUpdate(World* world, Player* player, BreakingState* breaking_state) {
    static i32 prevx = 0;
    static i32 prevy = 0;
    static i32 prevz = 0;
    const VECTOR player_chunk_pos = vec3_i32(
        ((player->physics_object.position.vx >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE) - (player->physics_object.position.vx < 0),
        ((player->physics_object.position.vy >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE) - (player->physics_object.position.vy < 0),
        ((player->physics_object.position.vz >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE) - (player->physics_object.position.vz < 0)
    );
    if (player_chunk_pos.vx != prevx
        || player_chunk_pos.vy != prevy
        || player_chunk_pos.vz != prevz) {
        prevx = player_chunk_pos.vx;
        prevy = player_chunk_pos.vy;
        prevz = player_chunk_pos.vz;
        // DEBUG_LOG("Player chunk pos: %d,%d,%d\n", VEC_LAYOUT(player_chunk_pos));
        worldLoadChunks(world, &player_chunk_pos);
        // DEBUG_LOG(
        //     "[WORLD] Head { x: %d, z: %d } Centre { x: %d, z: %d}\n",
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
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
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
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return false;
    }
    Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                [arrayCoord(world, vx, position->chunk.vx)]
                                [position->chunk.vy];
    if (chunk == NULL) {
        return false;
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
        return false;
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
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
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
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
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
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
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
    if (chunk == NULL) {
        return;
    }
    chunkRemoveLightValue(
        chunk,
        &position->block,
        light_type
    );
}

INLINE LightLevel worldGetInternalLightLevel(const World* world) {
    return world->internal_light_level;
}
