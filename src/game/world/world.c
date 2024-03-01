#include "world.h"

#include <assert.h>
#include <psxapi.h>
#include <psxgpu.h>

#include "../math/math_utils.h"
#include "../ui/progress_bar.h"
#include "../ui/background.h"

#define wrapCoord(world, axis, coord) positiveModulo(((world)->head.axis + (coord)), AXIS_CHUNKS)
#define arrayCoord(world, axis, value) wrapCoord(\
    world, \
    axis, \
    ((value) - (world->centre.axis - LOADED_CHUNKS_RADIUS - SHIFT_ZONE))\
)

// World should be loaded before invoking this method
void worldInit(World* world, RenderContext* ctx) {
    // Clear the chunks first to ensure they are all NULL upon initialisation
    for (int x = 0; x < AXIS_CHUNKS; x++) {
        for (int z = 0; z < AXIS_CHUNKS; z++) {
            for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                world->chunks[z][x][y] = NULL;
            }
        }
    }
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const int x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    ProgressBar bar = (ProgressBar) {
        .position = {
            .x = CENTRE_X - (CENTRE_X / 2),
            .y = CENTRE_Y - 2
        },
        .dimensions = {
            .width = CENTRE_X,
            .height = 4
        },
        .value = 0,
        .maximum = ((x_end + 1) - x_start) * ((z_end + 1) - z_start) * WORLD_CHUNKS_HEIGHT * 2
    };
    const int fnt_id = FntOpen(
        CENTRE_X - (CENTRE_X / 2),
        CENTRE_Y - 28,
        CENTRE_X,
        CENTRE_Y,
        0,
        100
    );
    printf("[WORLD] Loading chunks\n");
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                Chunk* chunk = worldLoadChunk(world, (VECTOR){
                                                  .vx = x,
                                                  .vy = y,
                                                  .vz = z
                                              });
                world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y] = chunk;
                FntPrint(
                    fnt_id,
                    "Loading World\n"
                );
                FntPrint(
                    fnt_id,
                    "Loading Chunk (%d,%d,%d)\n",
                    x, y, z
                );
                backgroundDraw(ctx, 2, 2 * BLOCK_TEXTURE_SIZE, 0 * BLOCK_TEXTURE_SIZE);
                bar.value++;
                progressBarRender(&bar, 1, ctx);
                FntFlush(fnt_id);
                swapBuffers(ctx);
            }
        }
    }
    printf("[WORLD] Building chunk meshes\n");
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                printf("[CHUNK: %d,%d,%d] Generating mesh\n", x, 0, z);
                Chunk* chunk = world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y];
                chunkGenerateMesh(chunk);
                printf(
                    "[CHUNK: %d,%d,%d, INDEX: %d,%d,%d] Mesh { Primitives: %d, Vertices: %d, Normals: %d }\n",
                    x, y, z,
                    arrayCoord(world, vx, x),
                    y,
                    arrayCoord(world, vz, z),
                    chunk->mesh.n_prims,
                    chunk->mesh.n_verts,
                    chunk->mesh.n_norms
                );
                FntPrint(
                    fnt_id,
                    "Loading World\n"
                );
                FntPrint(
                    fnt_id,
                    "Building Chunk Mesh (%d,%d,%d)\n",
                    x, y, z
                );
                backgroundDraw(ctx, 2, 2 * BLOCK_TEXTURE_SIZE, 0 * BLOCK_TEXTURE_SIZE);
                bar.value++;
                progressBarRender(&bar, 1, ctx);
                FntFlush(fnt_id);
                swapBuffers(ctx);
            }
        }
    }
    printf("[WORLD] Finished loading\n");
}

void worldDestroy(World* world) {
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const int x_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                printf(
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
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const int x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
                chunkRender(
                    world->chunks[arrayCoord(world, vz, z)][arrayCoord(world, vx, x)][y],
                    ctx,
                    transforms
                );
            }
        }
    }
}

// NOTE: Should this just take int32_t x,y,z params instead of a
//       a VECTOR struct to avoid creating needless stack objects?
Chunk* worldLoadChunk(World* world, const VECTOR chunk_position) {
    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk != NULL);
    printf(
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
    printf(
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

void worldLoadChunksX(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load x_direction chunks
    int32_t x_shift_zone = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    int32_t z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    int32_t z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    if (z_direction == -1) {
        z_end -= SHIFT_ZONE;
    } else if (z_direction == 1) {
        z_start += SHIFT_ZONE;
    }
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, (VECTOR){
                                              .vx = x_shift_zone,
                                              .vy = y,
                                              .vz = z_coord
                                          });
            chunkGenerateMesh(chunk);
            world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)][y] = chunk;
        }
    }
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)][y];
            chunkGenerateMesh(chunk);
        }
    }
    // Unload -x_direction chunks
    x_shift_zone = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk** chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)];
            worldUnloadChunk(world, chunk[y]);
            chunk[y] = NULL;
        }
    }
}

void worldLoadChunksZ(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load z_direction chunks
    int32_t z_shift_zone = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    int32_t x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;;
    int32_t x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    /* Can be simplified to:
     * int32_t x_start = world->centre.vx - LOADED_CHUNKS_RADIUS + ((x_direction == -1) * SHIFT_ZONE);
     * int32_t x_end = world->centre.vz + LOADED_CHUNKS_RADIUS - ((x_direction == 1) * SHIFT_ZONE);
     */
    if (x_direction == -1) {
        x_end -= SHIFT_ZONE;
    } else if (x_direction == 1) {
        x_start += SHIFT_ZONE;
    }
    printf("Z shift: %d\n", z_shift_zone);
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, (VECTOR){
                                              .vx = x_coord,
                                              .vy = y,
                                              .vz = z_shift_zone
                                          });
            printf("VZ: %d\n", arrayCoord(world, vz, z_shift_zone));
            world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)][y] = chunk;
        }
    }
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)][y];
            chunkGenerateMesh(chunk);
        }
    }
    // Unload -z_direction chunks
    z_shift_zone = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk** chunk = world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)];
            worldUnloadChunk(world, chunk[y]);
            chunk[y] = NULL;
        }
    }
}

void worldLoadChunksXZ(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load (x_direction,z_direction) chunk
    int32_t x_coord = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    int32_t z_coord = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk* loaded_chunk = worldLoadChunk(world, (VECTOR){
                                                 .vx = x_coord,
                                                 .vy = y,
                                                 .vz = z_coord
                                             });
        world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)][y] = loaded_chunk;
    }
    for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk* loaded_chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)][y];
        chunkGenerateMesh(loaded_chunk);
    }
    // Unload (-x_direction,-z_direction) chunk
    x_coord = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    z_coord = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
        Chunk** unloaded_chunk = world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)];
        worldUnloadChunk(world, unloaded_chunk[y]);
        unloaded_chunk[y] = NULL;
    }
}

void worldShiftChunks(World* world, const int8_t x_direction, const int8_t z_direction) {
    world->head.vx = wrapCoord(world, vx, x_direction);
    world->head.vz = wrapCoord(world, vz, z_direction);
    // Move centre towards player position by 1 increment
    world->centre.vx += x_direction;
    world->centre.vz += z_direction;
}

__attribute__((always_inline))
inline int worldWithinLoadRadius(const World* world, const VECTOR* player_chunk_pos) {
    return absv(world->centre.vx - player_chunk_pos->vx) < LOADED_CHUNKS_RADIUS - 1
           && absv(world->centre.vz - player_chunk_pos->vz) < LOADED_CHUNKS_RADIUS - 1;
}

void worldLoadChunks(World* world, const VECTOR* player_chunk_pos) {
    // Check if we need to load
    if (worldWithinLoadRadius(world, player_chunk_pos)) {
        return;
    }
    // Calculate direction shifts
    const int8_t x_direction = cmp(world->centre.vx, player_chunk_pos->vx);
    const int8_t z_direction = cmp(world->centre.vz, player_chunk_pos->vz);
    printf("x_dir: %d, z_dir: %d\n", x_direction, z_direction);
    // Load chunks
    if (x_direction != 0) {
        printf("X\n");
        worldLoadChunksX(world, x_direction, z_direction);
    }
    if (z_direction != 0) {
        printf("Z\n");
        worldLoadChunksZ(world, x_direction, z_direction);
    }
    if (x_direction != 0 && z_direction != 0) {
        printf("XZ\n");
        worldLoadChunksXZ(world, x_direction, z_direction);
    }
    printf("Shift chunks\n");
    // Shift chunks into centre of arrays
    worldShiftChunks(world, x_direction, z_direction);
}

void worldUpdate(World* world, const VECTOR* player_pos) {
    static int32_t prevx = 0;
    static int32_t prevy = 0;
    static int32_t prevz = 0;
    const VECTOR player_chunk_pos = (VECTOR){
        .vx = (player_pos->vx >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE,
        .vy = (player_pos->vy >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE,
        .vz = (player_pos->vz >> FIXED_POINT_SHIFT) / CHUNK_BLOCK_SIZE
    };
    if (player_chunk_pos.vx != prevx
        || player_chunk_pos.vy != prevy
        || player_chunk_pos.vz != prevz) {
        prevx = player_chunk_pos.vx;
        prevy = player_chunk_pos.vy;
        prevz = player_chunk_pos.vz;
        printf("Player chunk pos: %d,%d,%d\n", inlineVec(player_chunk_pos));
        worldLoadChunks(world, &player_chunk_pos);
        printf(
            "[WORLD] Head { x: %d, z: %d } Centre { x: %d, z: %d}\n",
            world->head.vx, world->head.vz,
            world->centre.vx, world->centre.vz
        );
        for (int z = 0; z < AXIS_CHUNKS; z++) {
            for (int x = 0; x < AXIS_CHUNKS; x++) {
                printf("%d ", world->chunks[z][x][0] != NULL);
            }
            printf("\n");
        }
    }
}

BlockID worldGetChunkBlock(const World* world, const ChunkBlockPosition* position) {
    // World is void below 0 on y-axis and nothing above height limit
    if ((position->chunk.vy <= 0 && position->block.vy < 0)
        || position->chunk.vy >= WORLD_CHUNKS_HEIGHT) {
        return BLOCKID_NONE;
    }
    const Chunk* chunk = world->chunks[arrayCoord(world, vz, position->chunk.vz)]
                                      [arrayCoord(world, vx, position->chunk.vx)]
                                      [position->chunk.vy];
    if (chunk == NULL) {
        return BLOCKID_NONE;
    }
    return chunkGetBlockVec(chunk, &position->block);
}

BlockID worldGetBlock(const World* world, const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        printf("[ERROR] Invalid Y: %d\n", position->vy);
        return BLOCKID_NONE;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    return worldGetChunkBlock(world, &chunk_block_position);
}

bool worldModifyVoxelChunkBlock(World* world, const ChunkBlockPosition* position, EBlockID block) {
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
    return chunkModifyVoxel(chunk, &position->block, block);
}

bool worldModifyVoxel(World* world, const VECTOR* position, EBlockID block) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        printf("[ERROR] Invalid Y: %d\n", position->vy);
        return false;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    printf(
        "Chunk: (%d,%d,%d) Block: (%d,%d,%d)\n",
        chunk_block_position.chunk.vx,
        chunk_block_position.chunk.vy,
        chunk_block_position.chunk.vz,
        chunk_block_position.block.vx,
        chunk_block_position.block.vy,
        chunk_block_position.block.vz
    );
    return worldModifyVoxelChunkBlock(world, &chunk_block_position, block);
}

int32_t intbound(const int32_t s, const int32_t ds) {
    printf("[intbound] s: %d, ds: %d\n", s, ds);
    if (ds < 0) {
        return intbound(-s, -ds);
    }
    printf("[intbound] end\n");
    return (((BLOCK_SIZE << FIXED_POINT_SHIFT) - positiveModulo(s, BLOCK_SIZE << FIXED_POINT_SHIFT)) << FIXED_POINT_SHIFT) / ds;
}

RayCastResult worldRayCastIntersection_new1(const World* world,
                                       const Camera* camera,
                                       const int32_t radius,
                                       cvector(SVECTOR*) markers) {
    VECTOR position = (VECTOR) {
        .vx = camera->position.vx,
        .vy = -camera->position.vy,
        .vz = camera->position.vz
    };
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const int32_t dx = direction.vx;
    const int32_t dy = direction.vy;
    const int32_t dz = direction.vz;
    printf("Direction: (%d,%d,%d)\n", inlineVec(direction));
    if (dx == 0 && dy == 0 && dz == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = BLOCKID_NONE,
            .face = {0}
        };
    }
    int32_t ix = (position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    int32_t iy = (position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    int32_t iz = (position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    printf("Index position: (%d,%d,%d)\n", ix, iy, iz);
    const int32_t step_x = sign(dx);
    const int32_t step_y = sign(dy);
    const int32_t step_z = sign(dz);
    printf("Step: (%d,%d,%d)\n", step_x, step_y, step_z);
    const int32_t tx_delta = absv((ONE << FIXED_POINT_SHIFT) / dx);
    const int32_t ty_delta = absv((ONE << FIXED_POINT_SHIFT) / dy);
    const int32_t tz_delta = absv((ONE << FIXED_POINT_SHIFT) / dz);
    printf("Delta: (%d,%d,%d)\n", tx_delta, ty_delta, tz_delta);
    const int32_t x_dist = step_x > 0 ? (((ix + 1) * BLOCK_SIZE) << FIXED_POINT_SHIFT) - position.vx : position.vx - ((ix * BLOCK_SIZE) << FIXED_POINT_SHIFT);
    const int32_t y_dist = step_y > 0 ? (((iy + 1) * BLOCK_SIZE) << FIXED_POINT_SHIFT) - position.vy : position.vy - ((iy * BLOCK_SIZE) << FIXED_POINT_SHIFT);
    const int32_t z_dist = step_z > 0 ? (((iz + 1) * BLOCK_SIZE) << FIXED_POINT_SHIFT) - position.vz : position.vz - ((iz * BLOCK_SIZE) << FIXED_POINT_SHIFT);
    printf("Dist: (%d,%d,%d)\n", x_dist, y_dist, z_dist);
    int32_t tx_max = fixedMulFrac(tx_delta, x_dist);
    int32_t ty_max = fixedMulFrac(ty_delta, y_dist);
    int32_t tz_max = fixedMulFrac(tz_delta, z_dist);
    printf("Max: (%d,%d,%d)\n", tx_max, ty_max, tz_max);
    int32_t stepped_index = -1;
    int32_t t = 0;
    const int32_t world_min_x = (world->centre.vx - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const int32_t world_max_x = (world->centre.vx + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World X [Min: %d] [Max: %d]\n", world_min_x, world_max_x);
    const int32_t world_min_y = 0;
    const int32_t world_max_y = WORLD_HEIGHT * BLOCK_SIZE;
    printf("World Y [Min: %d] [Max: %d]\n", world_min_y, world_max_y);
    const int32_t world_min_z = (world->centre.vz - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const int32_t world_max_z = (world->centre.vz + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
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
            const BlockID block = worldGetBlock(world, &temp_pos);
            printf("Queried block: (%d,%d,%d) = %d\n", inlineVec(temp_pos), block);
            if (block != BLOCKID_NONE && block != BLOCKID_AIR) {
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
                                       const int32_t radius,
                                       cvector(SVECTOR*) markers) {
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const int32_t dir_x = direction.vx;
    const int32_t dir_y = direction.vy;
    const int32_t dir_z = direction.vz;
    printf("Direction: (%d,%d,%d)\n", inlineVec(direction));
    if (dir_x == 0 && dir_y == 0 && dir_z == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = BLOCKID_NONE,
            .face = {0}
        };
    }
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t x1 = x = camera->position.vx;
    int32_t y1 = y = -camera->position.vy;
    int32_t z1 = z = camera->position.vz;
    int32_t x2 = x1 + (dir_x * radius);
    int32_t y2 = y1 + (dir_y * radius);
    int32_t z2 = z1 + (dir_z * radius);
    int dx = absv(x1 - x2);
    int dy = absv(y1 - y2);
    int dz = absv(z1 - z2);
    int32_t sx = x1 ? 1 : -1;
    int32_t sy = y1 ? 1 : -1;
    int32_t sz = z1 ? 1 : -1;
    if (dx >= dy && dx >= dz) {
        int32_t ey = (3 * dy) - dz;
        int32_t ez = (3 * dz) - dx;
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
                                       int32_t radius,
                                       cvector(SVECTOR)* markers) {
    // See: https://github.com/kpreid/cubes/blob/c5e61fa22cb7f9ba03cd9f22e5327d738ec93969/world.js#L307
    // See: http://www.cse.yorku.ca/~amana/research/grid.pdf
    VECTOR position = (VECTOR) {
        .vx = camera->position.vx,// / BLOCK_SIZE,
        .vy = -camera->position.vy,// / BLOCK_SIZE,
        .vz = camera->position.vz// / BLOCK_SIZE
    };
    printf("Origin: (%d,%d,%d)\n", inlineVec(position));
    printf("Before rotation to direction\n");
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const int32_t dx = direction.vx;
    const int32_t dy = direction.vy;
    const int32_t dz = direction.vz;
    printf("dx: %d, dy: %d, dz: %d\n", dx, dy, dz);
    if (dx == 0 && dy == 0 && dz == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = BLOCKID_NONE,
            .face = {0}
        };
    }
    const int32_t step_x = (sign(dx) << FIXED_POINT_SHIFT) * BLOCK_SIZE;
    const int32_t step_y = (sign(dy) << FIXED_POINT_SHIFT) * BLOCK_SIZE;
    const int32_t step_z = (sign(dz) << FIXED_POINT_SHIFT) * BLOCK_SIZE;
    printf("step: (%d,%d,%d)\n", step_x, step_y, step_z);
    printf("Before intbound\n");
    // TODO: Should these be relative to camera position instead of closest block position? (E.g. don't div by BLOCK_SIZE)
    int32_t t_max_x = intbound(position.vx, dx);
    int32_t t_max_y = intbound(position.vy, dy);
    int32_t t_max_z = intbound(position.vz, dz);
    printf("After intbound: (%d,%d,%d)\n", t_max_x, t_max_y, t_max_z);
    const int32_t t_delta_x = (step_x << FIXED_POINT_SHIFT) / dx;
    const int32_t t_delta_y = (step_y << FIXED_POINT_SHIFT) / dy;
    const int32_t t_delta_z = (step_z << FIXED_POINT_SHIFT) / dz;
    printf("t_delta: (%d,%d,%d)\n", t_delta_x, t_delta_y, t_delta_z);
    VECTOR face = (VECTOR) { .vx = 0, .vy = 0, .vz = 0 };
    // Rescale from units of 1 cube-edge to units of 'direction' so we can
    // compare with 't'.
    const int32_t world_min_x = (world->centre.vx - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const int32_t world_max_x = (world->centre.vx + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World X [Min: %d] [Max: %d]\n", world_min_x, world_max_x);
    const int32_t world_min_y = 0;
    const int32_t world_max_y = WORLD_HEIGHT * BLOCK_SIZE;
    printf("World Y [Min: %d] [Max: %d]\n", world_min_y, world_max_y);
    const int32_t world_min_z = (world->centre.vz - WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    const int32_t world_max_z = (world->centre.vz + WORLD_CHUNKS_RADIUS) * CHUNK_SIZE * BLOCK_SIZE;
    printf("World Z [Min: %d] [Max: %d]\n", world_min_z, world_max_z);
    printf("X: %d, Y: %d, Z: %d\n", position.vx >> FIXED_POINT_SHIFT, position.vy >> FIXED_POINT_SHIFT, position.vz >> FIXED_POINT_SHIFT);
    radius *= BLOCK_SIZE;
    while (1) {
        printf("[Raycast] Checking (%d,%d,%d)\n", position.vx >> FIXED_POINT_SHIFT, position.vy >> FIXED_POINT_SHIFT, position.vz >> FIXED_POINT_SHIFT);
        printf(
            "Out of world check [X: %d < %d || %d >= %d] [Y: %d < %d || %d >= %d] [Z: %d < %d || %d >= %d]\n",
            position.vx >> FIXED_POINT_SHIFT, world_min_x,
            position.vx >> FIXED_POINT_SHIFT, world_max_x,
            position.vy >> FIXED_POINT_SHIFT, world_min_y,
            position.vy >> FIXED_POINT_SHIFT, world_max_y,
            position.vz >> FIXED_POINT_SHIFT, world_min_z,
            position.vz >> FIXED_POINT_SHIFT, world_max_z
        );
        cvector_push_back((*markers), (SVECTOR) {});
        SVECTOR* cpos = &(*markers)[cvector_size((*markers)) - 1];
        cpos->vx = (((position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
        cpos->vy = (((-position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
        cpos->vz = (((position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
#define inWorld(_v) (step_##_v > 0 ? (position.v##_v >> FIXED_POINT_SHIFT) < world_max_##_v : (position.v##_v >> FIXED_POINT_SHIFT) >= world_min_##_v)
        if (inWorld(x) && inWorld(y) && inWorld(z)) {
#undef inWorld
            const VECTOR temp_pos = (VECTOR) {
                .vx = (position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
                .vy = (position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
                .vz = (position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT,
            };
            printf("Querying block: (%d,%d,%d)\n", inlineVec(temp_pos));
            const BlockID block = worldGetBlock(world, &temp_pos);
            printf("Querying block (back shift): (%d,%d,%d)\n", inlineVec(temp_pos));
            if (block != BLOCKID_NONE && block != BLOCKID_AIR) {
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
            .vx = position.vx / BLOCK_SIZE,
            .vy = position.vy / BLOCK_SIZE,
            .vz = position.vz / BLOCK_SIZE
        },
        .block = worldGetBlock(world, &intersection),
        .face = face
    };
}