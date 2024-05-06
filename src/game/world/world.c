#include "world.h"

#include <assert.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxgte.h>

#include "../../util/interface99_extensions.h"
#include "../render/font.h"
#include "../math/math_utils.h"
#include "../ui/progress_bar.h"
#include "../ui/background.h"
#include "../../debug/debug.h"

#define wrapCoord(world, axis, coord) positiveModulo(((world)->head.axis + (coord)), AXIS_CHUNKS)
#define arrayCoord(world, axis, value) wrapCoord(\
    world, \
    axis, \
    ((value) - (world->centre.axis - LOADED_CHUNKS_RADIUS - SHIFT_ZONE))\
)

// World should be loaded before invoking this method
void worldInit(World* world, RenderContext* ctx) {
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
    // TODO: Move text and progress bar to (0,0) and move to
    //       (CENTRE_X,CENTRE_Y) using translation vector and
    //       geometry matrix
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
        .maximum = ((x_end + 1) - x_start) * ((z_end + 1) - z_start) * WORLD_CHUNKS_HEIGHT * 2
    };
    DEBUG_LOG("[WORLD] Loading chunks\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                Chunk* chunk = worldLoadChunk(world, (VECTOR){
                                                  .vx = x,
                                                  .vy = y,
                                                  .vz = z
                                              });
                world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y] = chunk;
                fontPrintCentreOffset(
                    ctx,
                    CENTRE_X,
                    CENTRE_Y - ((FONT_SPRITE_HEIGHT + 2) * 3),
                    0,
                    "Loading World"
                );
                fontPrintCentreOffset(
                    ctx,
                    CENTRE_X,
                    CENTRE_Y - ((FONT_SPRITE_HEIGHT + 2) * 2) - 1,
                    10,
                    "Chunk [%d,%d,%d]",
                    x, y, z
                );
                fontPrintCentreOffset(
                    ctx,
                    CENTRE_X,
                    CENTRE_Y - ((FONT_SPRITE_HEIGHT + 2) * 1) - 1,
                    0,
                    "Loading Chunk Data"
                );
                backgroundDraw(ctx, 2, 2 * BLOCK_TEXTURE_SIZE, 0 * BLOCK_TEXTURE_SIZE);
                bar.value++;
                progressBarRender(&bar, 1, ctx);
                swapBuffers(ctx);
            }
        }
    }
    DEBUG_LOG("[WORLD] Building chunk meshes\n");
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                printf("[CHUNK: %d,%d,%d] Generating mesh\n", x, 0, z);
                Chunk* chunk = world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y];
                chunkGenerateMesh(chunk);
                DEBUG_LOG(
                    "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Mesh { Primitives: %d, Vertices: %d, Normals: %d }\n",
                    x, y, z,
                    arrayCoord(world, vx, x),
                    y,
                    arrayCoord(world, vz, z),
                    chunk->mesh.n_prims,
                    chunk->mesh.n_verts,
                    chunk->mesh.n_norms
                );
                fontPrintCentreOffset(
                    ctx,
                    CENTRE_X,
                    CENTRE_Y - ((FONT_SPRITE_HEIGHT + 2) * 3),
                    0,
                    "Loading World"
                );
                fontPrintCentreOffset(
                    ctx,
                    CENTRE_X,
                    CENTRE_Y - ((FONT_SPRITE_HEIGHT + 2) * 2) - 1,
                    10,
                    "Chunk [%d,%d,%d]",
                    x, y, z
                );
                fontPrintCentreOffset(
                    ctx,
                    CENTRE_X,
                    CENTRE_Y - ((FONT_SPRITE_HEIGHT + 2) * 1) - 1,
                    10,
                    "Building Mesh",
                    x, y, z
                );
                backgroundDraw(ctx, 2, 2 * BLOCK_TEXTURE_SIZE, 0 * BLOCK_TEXTURE_SIZE);
                bar.value++;
                progressBarRender(&bar, 1, ctx);
                swapBuffers(ctx);
            }
        }
    }
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
                Chunk** chunk = world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)];
                worldUnloadChunk(world, chunk[y]);
                chunk[y] = NULL;
            }
        }
    }
}

void worldRender(const World* world, RenderContext* ctx, Transforms* transforms) {
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
                    world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y],
                    ctx,
                    transforms
                );
            }
        }
    }
}

// NOTE: Should this just take i32 x,y,z params instead of a
//       a VECTOR struct to avoid creating needless stack objects?
Chunk* worldLoadChunk(World* world, const VECTOR chunk_position) {
    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk != NULL);
    DEBUG_LOG(
        "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Generating heightmap and terrain\n",
        chunk_position.vx,
        chunk_position.vy,
        chunk_position.vz,
        arrayCoord(world, vx, chunk_position.vx),
        chunk_position.vy,
        arrayCoord(world, vz, chunk_position.vz)
    );
    chunk->position.vx = chunk_position.vx;
    chunk->position.vy = chunk_position.vy;
    chunk->position.vz = chunk_position.vz;
    chunk->world = world;
    chunkInit(chunk);
    return chunk;
}

void worldUnloadChunk(const World* world, Chunk* chunk) {
    DEBUG_LOG(
        "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Unloading chunk\n",
        chunk->position.vx,
        chunk->position.vy,
        chunk->position.vz,
        arrayCoord(world, vx, chunk->position.vx),
        chunk->position.vy,
        arrayCoord(world, vz, chunk->position.vz)
    );
    chunkDestroy(chunk);
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
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, vec3_i32(x_shift_zone, y, z_coord));
            world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)][y] = chunk;
        }
    }
    // We need to generate the mesh after creating all the chunks
    // since each chunk needs to index into it's neighbours to
    // determine if a mesh face should be created on the border.
    // So if the neighbour doesn't exit yet we get more quads in
    // a chunk mesh than are actually necessary.
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)][y];
            chunkGenerateMesh(chunk);
        }
    }
    // Unload -x_direction chunks
    x_shift_zone = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    for (i32 z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk** chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)];
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
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, vec3_i32(x_coord, y, z_shift_zone));
            world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)][y] = chunk;
        }
    }
    // We need to generate the mesh after creating all the chunks
    // since each chunk needs to index into it's neighbours to
    // determine if a mesh face should be created on the border.
    // So if the neighbour doesn't exit yet we get more quads in
    // a chunk mesh than are actually necessary.
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)][y];
            chunkGenerateMesh(chunk);
        }
    }
    // Unload -z_direction chunks
    z_shift_zone = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    for (i32 x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk** chunk = world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)];
            worldUnloadChunk(world, chunk[y]);
            chunk[y] = NULL;
        }
    }
}

void worldLoadChunksXZ(World* world, const i8 x_direction, const i8 z_direction) {
    // Load (x_direction,z_direction) chunk
    i32 x_coord = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    i32 z_coord = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk* loaded_chunk = worldLoadChunk(world, vec3_i32(x_coord, y, z_coord));
        world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)][y] = loaded_chunk;
    }
    // We need to generate the mesh after creating all the chunks
    // since each chunk needs to index into it's neighbours to
    // determine if a mesh face should be created on the border.
    // So if the neighbour doesn't exit yet we get more quads in
    // a chunk mesh than are actually necessary.
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk* loaded_chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)][y];
        chunkGenerateMesh(loaded_chunk);
    }
    // Unload (-x_direction,-z_direction) chunk
    x_coord = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    z_coord = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk** unloaded_chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)];
        worldUnloadChunk(world, unloaded_chunk[y]);
        unloaded_chunk[y] = NULL;
    }
}

void worldShiftChunks(World* world, const i8 x_direction, const i8 z_direction) {
    DEBUG_LOG("[WORLD] Shift: set head\n");
    world->head.vx = wrapCoord(world, vx, x_direction);
    world->head.vz = wrapCoord(world, vz, z_direction);
    // Move centre towards player position by 1 increment
    DEBUG_LOG("[WORLD] Shift: set centre\n");
    world->centre.vx += x_direction;
    world->centre.vz += z_direction;
}

__attribute__((always_inline))
inline bool worldWithinLoadRadius(const World* world, const VECTOR* player_chunk_pos) {
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
    // Shift chunks into centre of arrays
    worldShiftChunks(world, x_direction, z_direction);
}

void worldUpdate(World* world, Player* player) {
    static i32 prevx = 0;
    static i32 prevy = 0;
    static i32 prevz = 0;
    const VECTOR player_chunk_pos = vec3_i32(
        (player->physics_object.position.vx >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE,
        (player->physics_object.position.vy >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE,
        (player->physics_object.position.vz >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE
    );
    if (player_chunk_pos.vx != prevx
        || player_chunk_pos.vy != prevy
        || player_chunk_pos.vz != prevz) {
        prevx = player_chunk_pos.vx;
        prevy = player_chunk_pos.vy;
        prevz = player_chunk_pos.vz;
        // BUG: Something weird is going on when moving between chunks and loading
        //      + unloading them. At times you can go beyond the boundary and then
        //      get a bad memory access error.
        DEBUG_LOG("Player chunk pos: %d,%d,%d\n", inlineVec(player_chunk_pos));
        worldLoadChunks(world, &player_chunk_pos);
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
    const i32 x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const i32 x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const i32 z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const i32 z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    for (i32 x = x_start; x <= x_end; x++) {
        for (i32 z = z_start; z <= z_end; z++) {
            for (i32 y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                chunkUpdate(
                    world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y],
                    player
                );
            }
        }
    }
}

IBlock* worldGetChunkBlock(const World* world, const ChunkBlockPosition* position) {
    // World is void below 0 on y-axis and nothing above height limit
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        // DEBUG_LOG(
        //     "[ERROR] Invalid Y [Chunk: %d] [Block: %d]\n",
        //     position->chunk.vy,
        //     position->block.vy
        // );
        return airBlockCreate();
    }
    const Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                      [arrayCoord(world, vx, position->chunk.vx)]
                                      [position->chunk.vy];
    if (chunk == NULL) {
        return airBlockCreate();
    }
    return chunkGetBlockVec(chunk, &position->block);
}

IBlock* worldGetBlock(const World* world, const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        // DEBUG_LOG("[ERROR] Invalid Y: %d\n", position->vy);
        return airBlockCreate();
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldGetChunkBlock(world, &chunk_block_position);
}

bool worldModifyVoxelChunkBlock(const World* world, const ChunkBlockPosition* position, IBlock* block, IItem** item_result) {
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
    return chunkModifyVoxel(chunk, &position->block, block, item_result);
}

bool worldModifyVoxel(const World* world, const VECTOR* position, IBlock* block, IItem** item_result) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        // printf("[ERROR] Invalid Y: %d\n", position->vy);
        return false;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    DEBUG_LOG(
        "Chunk: (%d,%d,%d) Block: (%d,%d,%d)\n",
        chunk_block_position.chunk.vx,
        chunk_block_position.chunk.vy,
        chunk_block_position.chunk.vz,
        chunk_block_position.block.vx,
        chunk_block_position.block.vy,
        chunk_block_position.block.vz
    );
    return worldModifyVoxelChunkBlock(world, &chunk_block_position, block, item_result);
}

i32 intbound(const i32 s, const i32 ds) {
    printf("[intbound] s: %d, ds: %d\n", s, ds);
    if (ds < 0) {
        return intbound(-s, -ds);
    }
    printf("[intbound] end\n");
    return (((BLOCK_SIZE << FIXED_POINT_SHIFT) - positiveModulo(s, BLOCK_SIZE << FIXED_POINT_SHIFT)) << FIXED_POINT_SHIFT) / ds;
}

RayCastResult worldRayCastIntersection_new1(const World* world,
                                       const Camera* camera,
                                       const i32 radius,
                                       cvector(SVECTOR*) markers) {
    const VECTOR position = vec3_i32(
        camera->position.vx,
        -camera->position.vy,
        camera->position.vz
    );
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const i32 dx = direction.vx;
    const i32 dy = direction.vy;
    const i32 dz = direction.vz;
    printf("Direction: (%d,%d,%d)\n", inlineVec(direction));
    if (dx == 0 && dy == 0 && dz == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = NULL,
            .face = {0}
        };
    }
    i32 ix = (position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    i32 iy = (position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    i32 iz = (position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    printf("Index position: (%d,%d,%d)\n", ix, iy, iz);
    const i32 step_x = sign(dx);
    const i32 step_y = sign(dy);
    const i32 step_z = sign(dz);
    printf("Step: (%d,%d,%d)\n", step_x, step_y, step_z);
    const i32 tx_delta = absv((ONE << FIXED_POINT_SHIFT) / dx);
    const i32 ty_delta = absv((ONE << FIXED_POINT_SHIFT) / dy);
    const i32 tz_delta = absv((ONE << FIXED_POINT_SHIFT) / dz);
    printf("Delta: (%d,%d,%d)\n", tx_delta, ty_delta, tz_delta);
    const i32 x_dist = step_x > 0 ? (((ix + 1) * BLOCK_SIZE) << FIXED_POINT_SHIFT) - position.vx : position.vx - ((ix * BLOCK_SIZE) << FIXED_POINT_SHIFT);
    const i32 y_dist = step_y > 0 ? (((iy + 1) * BLOCK_SIZE) << FIXED_POINT_SHIFT) - position.vy : position.vy - ((iy * BLOCK_SIZE) << FIXED_POINT_SHIFT);
    const i32 z_dist = step_z > 0 ? (((iz + 1) * BLOCK_SIZE) << FIXED_POINT_SHIFT) - position.vz : position.vz - ((iz * BLOCK_SIZE) << FIXED_POINT_SHIFT);
    printf("Dist: (%d,%d,%d)\n", x_dist, y_dist, z_dist);
    i32 tx_max = fixedMul(tx_delta, x_dist);
    i32 ty_max = fixedMul(ty_delta, y_dist);
    i32 tz_max = fixedMul(tz_delta, z_dist);
    printf("Max: (%d,%d,%d)\n", tx_max, ty_max, tz_max);
    i32 stepped_index = -1;
    i32 t = 0;
    const i32 world_min_x = (world->centre.vx - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const i32 world_max_x = (world->centre.vx + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World X [Min: %d] [Max: %d]\n", world_min_x, world_max_x);
    const i32 world_min_y = 0;
    const i32 world_max_y = WORLD_HEIGHT * BLOCK_SIZE;
    printf("World Y [Min: %d] [Max: %d]\n", world_min_y, world_max_y);
    const i32 world_min_z = (world->centre.vz - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const i32 world_max_z = (world->centre.vz + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World Z [Min: %d] [Max: %d]\n", world_min_z, world_max_z);
    VECTOR hit_position = (VECTOR) {0};
    VECTOR hit_norm = (VECTOR) {0};
    while (t < radius) {
        printf("t (%d) < radius (%d)\n", t, radius);
#define inWorld(_v) (step_##_v > 0 ? (position.v##_v >> FIXED_POINT_SHIFT) < world_max_##_v : (position.v##_v >> FIXED_POINT_SHIFT) >= world_min_##_v)
        if (inWorld(x) && inWorld(y) && inWorld(z)) {
#undef inWorld
            const VECTOR temp_pos = (VECTOR) {
                .vx = ix,
                .vy = iy,
                .vz = iz,
            };
            cvector_push_back((*markers), (SVECTOR) {});
            SVECTOR* cpos = &(*markers)[cvector_size((*markers)) - 1];
            cpos->vx = (ix * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
            cpos->vy = (-iy * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
            cpos->vz = (iz * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
            const Block* block = VCAST(Block*, *worldGetBlock(world, &temp_pos));
            printf("Queried block: (%d,%d,%d) = %d\n", inlineVec(temp_pos), block->id);
            if (block->id != BLOCKID_AIR) {
                hit_position.vx = position.vx + ((t * dx) >> FIXED_POINT_SHIFT);
                hit_position.vy = position.vy + ((t * dy) >> FIXED_POINT_SHIFT);
                hit_position.vz = position.vz + ((t * dz) >> FIXED_POINT_SHIFT);
                hit_norm.vx = hit_norm.vy = hit_norm.vz = 0;
                if (stepped_index == 0) {
                    hit_norm.vx = -step_x;
                } else if (stepped_index == 1) {
                    hit_norm.vy = -step_y;
                } else if (stepped_index == 2) {
                    hit_norm.vz = -step_z;
                }
                break;
            }
            // advance t to next nearest voxel boundary
            if (tx_max < ty_max) {
                if (tx_max < tz_max) {
                    ix += step_x;
                    t = tx_max;
                    tx_max += tx_delta;
                    stepped_index = 0;
                } else {
                    iz += step_z;
                    t = tz_max;
                    tz_max += tz_delta;
                    stepped_index = 2;
                }
            } else {
                if (ty_max < tz_max) {
                    iy += step_y;
                    t = ty_max;
                    ty_max += ty_delta;
                    stepped_index = 1;
                } else {
                    iz += step_z;
                    t = tz_max;
                    tz_max += tz_delta;
                    stepped_index = 2;
                }
            }
        }
    }
    const VECTOR intersection = (VECTOR) {
        .vx = (hit_position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
        .vy = (hit_position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
        .vz = (hit_position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT
    };
    printf("Position: (%d,%d,%d)\n", inlineVec(intersection));
    printf("Before getting block\n");
    return (RayCastResult) {
        .pos = vector_const_div(hit_position, BLOCK_SIZE),
        .block = worldGetBlock(world, &intersection),
        .face = hit_norm
    };
}

RayCastResult worldRayCastIntersection_new(const World* world,
                                       const Camera* camera,
                                       const i32 radius,
                                       cvector(SVECTOR*) markers) {
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const i32 dir_x = direction.vx;
    const i32 dir_y = direction.vy;
    const i32 dir_z = direction.vz;
    printf("Direction: (%d,%d,%d)\n", inlineVec(direction));
    if (dir_x == 0 && dir_y == 0 && dir_z == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = NULL,
            .face = {0}
        };
    }
    i32 x;
    i32 y;
    i32 z;
    const i32 x1 = x = camera->position.vx;
    const i32 y1 = y = -camera->position.vy;
    const i32 z1 = z = camera->position.vz;
    const i32 x2 = x1 + (dir_x * radius);
    const i32 y2 = y1 + (dir_y * radius);
    const i32 z2 = z1 + (dir_z * radius);
    const int dx = absv(x1 - x2);
    const int dy = absv(y1 - y2);
    const int dz = absv(z1 - z2);
    const i32 sx = x1 ? 1 : -1;
    const i32 sy = y1 ? 1 : -1;
    const i32 sz = z1 ? 1 : -1;
    if (dx >= dy && dx >= dz) {
        i32 ey = (3 * dy) - dz;
        i32 ez = (3 * dz) - dx;
        for (int i = 0; i < dx; i++) {
            x += sx;
            // TODO: Check intersection (x,y,z)
            if (ey > 0 && ez > 0) {
                if (ey * dz > ez * dy) {
                    y += sy;
                    // TODO: Check intersection (x,y,z)
                    z += sz;
                    // TODO: Check intersection (x,y,z)
                } else {
                    z += sz;
                    // TODO: Check intersection (x,y,z)
                    y += sy;
                    // TODO: Check intersection (x,y,z)
                }
                ey += 2 * (dy - dx);
                ez += 2 * (dz - dx);
            } else {
                if (ey > 0) {
                    y += sy;
                    // TODO: Check intersection (x,y,z)
                    ey += 2 * (dy - dx);
                    ez += 2 * dz;
                } else {
                    ey += 2 * dy;
                    if (ez > 0) {
                        z += sz;
                        // TODO: Check intersection (x,y,z)
                        ez += 2 * (dz - dx);
                    } else {
                        ez += 2 * dz;
                    }
                }
            }
        }
    } else if (dy >= dx && dy >= dz) {
        // y is the driving axis
    } else {
        // z is the driving axis
    }
    return (RayCastResult) {};
}

RayCastResult worldRayCastIntersection(const World* world,
                                       const Camera* camera,
                                       i32 radius,
                                       cvector(SVECTOR)* markers) {
    // See: https://github.com/kpreid/cubes/blob/c5e61fa22cb7f9ba03cd9f22e5327d738ec93969/world.js#L307
    // See: http://www.cse.yorku.ca/~amana/research/grid.pdf
    VECTOR position = vec3_i32(
        camera->position.vx,// / BLOCK_SIZE,
        -camera->position.vy,// / BLOCK_SIZE,
        camera->position.vz// / BLOCK_SIZE
    );
    printf("Origin: (%d,%d,%d)\n", inlineVec(position));
    printf("Before rotation to direction\n");
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const i32 dx = direction.vx;
    const i32 dy = direction.vy;
    const i32 dz = direction.vz;
    printf("dx: %d, dy: %d, dz: %d\n", dx, dy, dz);
    if (dx == 0 && dy == 0 && dz == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = NULL,
            .face = {0}
        };
    }
    const i32 step_x = (sign(dx) << FIXED_POINT_SHIFT) * BLOCK_SIZE;
    const i32 step_y = (sign(dy) << FIXED_POINT_SHIFT) * BLOCK_SIZE;
    const i32 step_z = (sign(dz) << FIXED_POINT_SHIFT) * BLOCK_SIZE;
    printf("step: (%d,%d,%d)\n", step_x, step_y, step_z);
    printf("Before intbound\n");
    i32 t_max_x = intbound(position.vx, dx);
    i32 t_max_y = intbound(position.vy, dy);
    i32 t_max_z = intbound(position.vz, dz);
    printf("After intbound: (%d,%d,%d)\n", t_max_x, t_max_y, t_max_z);
    const i32 t_delta_x = (step_x << FIXED_POINT_SHIFT) / dx;
    const i32 t_delta_y = (step_y << FIXED_POINT_SHIFT) / dy;
    const i32 t_delta_z = (step_z << FIXED_POINT_SHIFT) / dz;
    printf("t_delta: (%d,%d,%d)\n", t_delta_x, t_delta_y, t_delta_z);
    VECTOR face = (VECTOR) { .vx = 0, .vy = 0, .vz = 0 };
    // Rescale from units of 1 cube-edge to units of 'direction' so we can
    // compare with 't'.
    const i32 world_min_x = (world->centre.vx - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const i32 world_max_x = (world->centre.vx + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World X [Min: %d] [Max: %d]\n", world_min_x, world_max_x);
    const i32 world_min_y = 0;
    const i32 world_max_y = WORLD_HEIGHT * BLOCK_SIZE;
    printf("World Y [Min: %d] [Max: %d]\n", world_min_y, world_max_y);
    const i32 world_min_z = (world->centre.vz - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const i32 world_max_z = (world->centre.vz + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World Z [Min: %d] [Max: %d]\n", world_min_z, world_max_z);
    printf("X: %d, Y: %d, Z: %d\n", position.vx >> FIXED_POINT_SHIFT, position.vy >> FIXED_POINT_SHIFT, position.vz >> FIXED_POINT_SHIFT);
    radius *= BLOCK_SIZE;
    while (1) {
        printf("[Raycast] Checking (%d,%d,%d)\n", position.vx >> FIXED_POINT_SHIFT, position.vy >> FIXED_POINT_SHIFT, position.vz >> FIXED_POINT_SHIFT);
        cvector_push_back((*markers), (SVECTOR) {});
        SVECTOR* cpos = &(*markers)[cvector_size((*markers)) - 1];
        cpos->vx = (((position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
        cpos->vy = (((-position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
        cpos->vz = (((position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
#define inWorld(_v) (step_##_v > 0 ? (position.v##_v >> FIXED_POINT_SHIFT) < world_max_##_v : (position.v##_v >> FIXED_POINT_SHIFT) >= world_min_##_v)
        printf("In world: [X: %d] [Y: %d] [Z: %d]\n", inWorld(x), inWorld(y), inWorld(z));
        if (inWorld(x) && inWorld(y) && inWorld(z)) {
#undef inWorld
            const VECTOR temp_pos = (VECTOR) {
                .vx = (position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
                .vy = (position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
                .vz = (position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
            };
            const Block* block = VCAST(Block*, *worldGetBlock(world, &temp_pos));
            printf("Querying block: (%d,%d,%d) = %s\n", inlineVec(temp_pos), blockIdStringify(block->id));
            if (block->id != BLOCKID_AIR && block->type != BLOCKTYPE_EMPTY) {
                break;
            }
        }
        // tMaxX stores the t-value at which we cross a cube boundary along the
        // X axis, and similarly for Y and Z. Therefore, choosing the least tMax
        // chooses the closest cube boundary. Only the first case of the four
        // has been commented in detail.
        printf("t_max: [X: %d] [Y: %d] [Z: %d]\n", t_max_x, t_max_y, t_max_z);
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                printf("x < y && x < z\n");
                if (t_max_x > radius) {
                    printf("Exceeds radius on x: %d > %d\n", t_max_x, radius);
                    break;
                }
                // Update which cube we are now in.
                printf("%d + %d\n", position.vx, step_x);
                position.vx += step_x;
                // Adjust tMaxX to the next X-oriented boundary crossing.
                t_max_x += t_delta_x;
                // Record the normal vector of the cube face we entered.
                face.vx = -step_x;
                face.vy = 0;
                face.vz = 0;
            } else {
                printf("x < y && x >= z\n");
                if (t_max_z > radius) {
                    printf("Exceeds radius on z: %d > %d\n", t_max_z, radius);
                    break;
                }
                printf("%d + %d\n", position.vz, step_z);
                position.vz += step_z;
                t_max_z += t_delta_z;
                face.vx = 0;
                face.vy = 0;
                face.vz = -step_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                printf("x >= y && y < z\n");
                if (t_max_y > radius) {
                    printf("Exceeds radius on y: %d > %d\n", t_max_y, radius);
                    break;
                }
                printf("%d + %d\n", position.vy, step_y);
                position.vy += step_y;
                t_max_y += t_delta_y;
                face.vx = 0;
                face.vy = -step_y;
                face.vz = 0;
            } else {
                printf("x >= y && y >= z\n");
                // Identical to the second case, repeated for simplicity in
                // the conditionals.
                if (t_max_z > radius) {
                    printf("Exceeds radius on z (2): %d > %d\n", t_max_z, radius);
                    break;
                }
                printf("%d + %d\n", position.vz, step_z);
                position.vz += step_z;
                t_max_z += t_delta_z;
                face.vx = 0;
                face.vy = 0;
                face.vz = -step_z;
            }
        }
    }
    const VECTOR intersection = (VECTOR) {
        .vx = (position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
        .vy = (position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
        .vz = (position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT
    };
    printf("Position: (%d,%d,%d)\n", inlineVec(intersection));
    printf("Before getting block\n");
    return (RayCastResult) {
        .pos = (VECTOR) {
            .vx = position.vx,
            .vy = position.vy,
            .vz = position.vz
        },
        .block = worldGetBlock(world, &intersection),
        .face = face
    };
}
