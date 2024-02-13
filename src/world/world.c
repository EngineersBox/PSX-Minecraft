#include "world.h"

#include <assert.h>
#include <math_utils.h>

#define wrapCoord(world, axis, coord) positiveModulo(((world)->head.axis + (coord)), AXIS_CHUNKS)
#define arrayCoord(world, axis, value) wrapCoord(\
    world, \
    axis, \
    ((value) - (world->centre.axis - LOADED_CHUNKS_RADIUS - SHIFT_ZONE))\
)

// World should be loaded before invoking this method
void worldInit(World* world) {
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
    const int x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
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

    // TODO: Fix rendering of newly loaded chunks
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
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
            Chunk* chunk = worldLoadChunk(world, (VECTOR){
                                              .vx = x_coord,
                                              .vy = y,
                                              .vz = z_shift_zone
                                          });
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
}

__attribute__((always_inline))
inline int worldWithinLoadRadius(const World* world, const VECTOR* player_chunk_pos) {
    return absv(world->centre.vx - player_chunk_pos->vx) < LOADED_CHUNKS_RADIUS - 1
// #if LOADED_CHUNKS_RADIUS == 1
//            < LOADED_CHUNKS_RADIUS
// #else
//            < LOADED_CHUNKS_RADIUS - 1
// #endif
           && absv(world->centre.vz - player_chunk_pos->vz) < LOADED_CHUNKS_RADIUS - 1
// #if LOADED_CHUNKS_RADIUS == 1
//            < LOADED_CHUNKS_RADIUS
// #else
//            < LOADED_CHUNKS_RADIUS - 1
// #endif
        ;
}

void worldLoadChunks(World* world, const VECTOR* player_chunk_pos) {
    // Check if we need to load
    if (worldWithinLoadRadius(world, player_chunk_pos)) {
        return;
    }
    // Calculate direction shifts
    const int8_t x_direction = cmp(world->centre.vx, player_chunk_pos->vx);
    const int8_t z_direction = cmp(world->centre.vz, player_chunk_pos->vz);
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
    // Move centre towards player position by 1 increment
    world->centre.vx += x_direction;
    world->centre.vz += z_direction;
}

void worldUpdate(World* world, const VECTOR* player_pos) {
    // TODO: These are temp testing static vars, remove them later
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
        printf("[WORLD] Head { x: %d, z: %d }\n", world->head.vx, world->head.vz);
        for (int x = 0; x < AXIS_CHUNKS; x++) {
            for (int z = 0; z < AXIS_CHUNKS; z++) {
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
    return chunk->blocks[chunkBlockIndex(position->block.vx, position->block.vy, position->block.vz)];
}

BlockID worldGetBlock(const World* world, const VECTOR* position) {
    // World is void below 0 and above world-height on y-axis
    if (position->vy < 0 || position->vy >= WORLD_HEIGHT) {
        // printf("Invalid Y: %d\n", position->vy);
        return BLOCKID_NONE;
    }
    const ChunkBlockPosition chunk_block_position = worldToChunkBlockPosition(position, CHUNK_SIZE);
    // printf(
    //     "CB POS: [C: (%d,%d,%d)] [B: (%d,%d,%d)]\n",
    //     chunk_block_position.chunk.vx,
    //     chunk_block_position.chunk.vy,
    //     chunk_block_position.chunk.vz,
    //     chunk_block_position.block.vx,
    //     chunk_block_position.block.vy,
    //     chunk_block_position.block.vz
    // );
    return worldGetChunkBlock(world, &chunk_block_position);
}

int32_t intbound(const int32_t s, const int32_t ds) {
    printf("[intbound] s: %d, ds: %d\n", s, ds);
    if (ds < 0) {
        return intbound(-s, -ds);
    }
    printf("[intbound] end\n");
    return (ONE - positiveModulo(s, 1)) / ds;
}

int32_t signum(const int32_t x) {
    if (x > 0) {
        return 1;
    }
    return x < 0 ? -1 : 0;
}

RayCastResult worldRayCastIntersection(const World* world, const Camera* camera, int32_t radius) {
    // See: https://github.com/kpreid/cubes/blob/c5e61fa22cb7f9ba03cd9f22e5327d738ec93969/world.js#L307
    // See: http://www.cse.yorku.ca/~amana/research/grid.pdf
    int32_t x = camera->position.vx / BLOCK_SIZE;
    int32_t y = camera->position.vy / BLOCK_SIZE;
    int32_t z = camera->position.vz / BLOCK_SIZE;
    printf("Before rotation to direction\n");
    const VECTOR direction = rotationToDirection(&camera->rotation);
    const int32_t dx = direction.vx;
    const int32_t dy = direction.vy;
    const int32_t dz = direction.vz;
    printf("dx: %d, dy: %d, dz: %d\n", dx, dy, dz);
    const int32_t step_x = sign(dx);
    const int32_t step_y = sign(dy);
    const int32_t step_z = sign(dz);
    printf("step: (%d,%d,%d)\n", step_x, step_y, step_z);
    printf("Before intbound\n");
    int32_t t_max_x = intbound(x, dx);
    int32_t t_max_y = intbound(y, dy);
    int32_t t_max_z = intbound(z, dz);
    printf("After intbound: (%d,%d,%d)\n", t_max_x, t_max_y, t_max_z);
    int32_t t_delta_x = step_x / dx;
    int32_t t_delta_y = step_y / dy;
    int32_t t_delta_z = step_z / dz;
    printf("t_delta: (%d,%d,%d)\n", t_delta_x, t_delta_y, t_delta_z);
    VECTOR face = {};
    if (dx == 0 && dy == 0 && dz == 0) {
        printf("Zero delta\n");
        return (RayCastResult) {
            .pos = {0},
            .block = BLOCKID_NONE,
            .face = {0}
        };
    }
    // Rescale from units of 1 cube-edge to units of 'direction' so we can
    // compare with 't'.
    printf("Before sqrt12\n");
    radius /= SquareRoot12(dx * dx + dy * dy + dz * dz);
    const int32_t world_min_x = (world->centre.vx - LOADED_CHUNKS_RADIUS) * CHUNK_SIZE;
    const int32_t world_max_x = (world->centre.vx + LOADED_CHUNKS_RADIUS) * CHUNK_SIZE;
    const int32_t world_min_y = (world->centre.vy - LOADED_CHUNKS_RADIUS) * CHUNK_SIZE;
    const int32_t world_max_y = (world->centre.vy + LOADED_CHUNKS_RADIUS) * CHUNK_SIZE;
    const int32_t world_min_z = (world->centre.vz - LOADED_CHUNKS_RADIUS) * CHUNK_SIZE;
    const int32_t world_max_z = (world->centre.vz + LOADED_CHUNKS_RADIUS) * CHUNK_SIZE;

#define inWorld(v) (step_##v > 0 ? v < world_max_##v : v >= world_min_##v)
    while (inWorld(x) && inWorld(y) && inWorld(z)) {
#undef inWorld
#define outWorld(v) (v < world_min_##v || v >= world_max_##v)
        if (!(outWorld(x) || outWorld(y) || outWorld(z))) {
#undef outWorld
            break;
        }
        // tMaxX stores the t-value at which we cross a cube boundary along the
        // X axis, and similarly for Y and Z. Therefore, choosing the least tMax
        // chooses the closest cube boundary. Only the first case of the four
        // has been commented in detail.
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                if (t_max_x > radius) break;
                // Update which cube we are now in.
                x += step_x;
                // Adjust tMaxX to the next X-oriented boundary crossing.
                t_max_x += t_delta_x;
                // Record the normal vector of the cube face we entered.
                face.vx = -step_x;
                face.vy = 0;
                face.vz = 0;
            } else {
                if (t_max_z > radius) break;
                z += step_z;
                t_max_z += t_delta_z;
                face.vx = 0;
                face.vy = 0;
                face.vz = -step_z;
            }
        } else {
            if (t_max_y < t_max_z) {
                if (t_max_y > radius) break;
                y += step_y;
                t_max_y += t_delta_y;
                face.vx = 0;
                face.vy = -step_y;
                face.vz = 0;
            } else {
                // Identical to the second case, repeated for simplicity in
                // the conditionals.
                if (t_max_z > radius) break;
                z += step_z;
                t_max_z += t_delta_z;
                face.vx = 0;
                face.vy = 0;
                face.vz = -step_z;
            }
        }
    }
    const VECTOR intersection = (VECTOR) {
        .vx = x,
        .vy = y,
        .vz = z
    };
    printf("Before getting block\n");
    return (RayCastResult) {
        .pos = intersection,
        .block = worldGetBlock(world, &intersection),
        .face = face
    };
}