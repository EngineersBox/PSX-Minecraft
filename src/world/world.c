#include "world.h"

#include <math_utils.h>

#define positiveModulo(i, n) ((i % n + n) % n)
#define wrapCoord(world, axis, coord) positiveModulo(((world)->head.axis + (coord)), AXIS_CHUNKS)
#define arrayCoord(world, axis, value) wrapCoord(\
    world, \
    axis, \
    (value - (world->centre.axis - LOADED_CHUNKS_RADIUS - SHIFT_ZONE))\
)

// World should be loaded before invoking this method
void worldInit(World* world) {
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    const int x_end = world->centre.vz + LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            printf("[WORLD] LOADING CHUNK (%d,%d)\n", x, z);
            world->chunks[arrayCoord(world, vx, x)][arrayCoord(world, vz, z)][0] = worldLoadChunk((VECTOR) {
                .vx = x,
                .vy = 0, // What should this be?
                .vz = z
            });
        }
    }
}

void worldDestroy(World* world) {
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    const int x_end = world->centre.vz + LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            Chunk** chunk = world->chunks[arrayCoord(world, vx, x)][arrayCoord(world, vz, z)];
            worldUnloadChunk(chunk[0]);
            *chunk = NULL;
        }
    }
}

void worldRender(const World *world, DisplayContext *ctx, Transforms *transforms) {
    // TODO: Revamp with BFS for visible chunks occlusion (use frustum culling too?)
    const int x_start = world->centre.vx - LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    const int x_end = world->centre.vz + LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
    const int z_start = world->centre.vz - LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    const int z_end = world->centre.vz + LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
    for (int x = x_start; x <= x_end; x++) {
        for (int z = z_start; z <= z_end; z++) {
            chunkRender(
                world->chunks[arrayCoord(world, vx, x)][arrayCoord(world, vz, z)][0],
                ctx,
                transforms
            );
        }
    }
}

// NOTE: Should this just take int32_t x,y,z params instead of a
//       a VECTOR struct to avoid creating needless stack objects?
Chunk* worldLoadChunk(const VECTOR chunk_position) {
    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk != NULL);
    chunk->position.vx = chunk_position.vx;
    chunk->position.vy = chunk_position.vy;
    chunk->position.vz = chunk_position.vz;
    chunkInit(chunk);
    return chunk;
}

void worldUnloadChunk(Chunk* chunk) {
    chunkDestroy(chunk);
    free(chunk);
}

void worldLoadChunksX(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load x_direction chunks
    int32_t x_shift_zone = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    int32_t z_start;
    int32_t z_end;
    if (z_direction == -1) {
        z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
        z_end = world->centre.vz + LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    } else if (z_direction == 1) {
        z_start = world->centre.vz - LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
        z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    } else {
        z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
        z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
    }
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        Chunk* chunk = worldLoadChunk((VECTOR){
            .vx = x_shift_zone,
            .vy = 0, // What should this be?
            .vz = z_coord
        });
        world->chunks[arrayCoord(world, vx, x_shift_zone)][arrayCoord(world, vz, z_coord)][0] = chunk;
    }
    // Unload -x_direction chunks
    x_shift_zone = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        Chunk** chunk = world->chunks[arrayCoord(world, vx, x_shift_zone)][arrayCoord(world, vz, z_coord)];
        worldUnloadChunk(chunk[0]);
        *chunk = NULL;
    }
}

void worldLoadChunksZ(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load z_direction chunks
    int32_t z_shift_zone = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    int32_t x_start;
    int32_t x_end;
    if (x_direction == -1) {
        x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
        x_end = world->centre.vx + LOADED_CHUNKS_RADIUS - SHIFT_ZONE;
    } else if (x_direction == 1) {
        x_start = world->centre.vx - LOADED_CHUNKS_RADIUS + SHIFT_ZONE;
        x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    } else {
        x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
        x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
    }
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        Chunk* chunk = worldLoadChunk((VECTOR){
            .vx = x_coord,
            .vy = 0, // What should this be?
            .vz = z_shift_zone
        });
        world->chunks[arrayCoord(world, vx, x_coord)][arrayCoord(world, vz, z_shift_zone)][0] = chunk;
    }
    // Unload -z_direction chunks
    z_shift_zone = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        Chunk** chunk = world->chunks[arrayCoord(world, vx, x_coord)][arrayCoord(world, vz, z_shift_zone)];
        worldUnloadChunk(chunk[0]);
        *chunk = NULL;
    }
}

void worldLoadChunksXZ(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load (x_direction,z_direction) chunk
    int32_t x_coord = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
    int32_t z_coord = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
    Chunk* loaded_chunk = worldLoadChunk((VECTOR) {
        .vx = x_coord,
        .vy = 0, // What should this be?
        .vz = z_coord
    });
    world->chunks[arrayCoord(world, vx, x_coord)][arrayCoord(world, vz, z_coord)][0] = loaded_chunk;
    // Unload (-x_direction,-z_direction) chunk
    x_coord = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
    z_coord = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
    Chunk** unloaded_chunk = world->chunks[arrayCoord(world, vx, x_coord)][arrayCoord(world, vz, z_coord)];
    worldUnloadChunk(unloaded_chunk[0]);
    *unloaded_chunk = NULL;
}

void worldShiftChunks(World* world, const int8_t x_direction, const int8_t z_direction) {
    world->head.vx = wrapCoord(world, vx, x_direction);
    world->head.vz = wrapCoord(world, vz, z_direction);
}

__attribute__((always_inline))
inline int worldWithinLoadRadius(const World* world, const VECTOR* player_pos) {
    return absv(world->centre.vx - player_pos->vx) < LOADED_CHUNKS_RADIUS - 1
        && absv(world->centre.vz - player_pos->vz) < LOADED_CHUNKS_RADIUS - 1;
}

__attribute__((always_inline))
inline int relativeDirection(const int32_t from, const int32_t to) {
    if (to == from) {
        return 0;
    }
    if (to < from) {
        return -1;
    }
    return 1;
}

void worldLoadChunks(World* world, const VECTOR* player_pos) {
    // Check if we need to load
    if (worldWithinLoadRadius(world, player_pos)) {
        return;
    }
    // Calculate direction shifts
    const int8_t x_direction = cmp(world->centre.vx, player_pos->vx);
    const int8_t z_direction = cmp(world->centre.vz, player_pos->vz);
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

BlockID worldGetChunkBlock(const ChunkBlockPosition *position) {
    // TODO: Implement this
}

BlockID worldGetBlock(const VECTOR *position) {
    // TODO: Implement this
}
