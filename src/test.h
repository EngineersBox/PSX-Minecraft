#include <stdio.h>
#include <inttypes.h>

#define sign(v) (((v) > 0) - ((v) < 0))
#define absv(v) (v * sign(v))

#define RADIUS 2
#define CENTRE 1
#define SHIFT_ZONE 1
#define AXIS_SIZE (((RADIUS + SHIFT_ZONE) * 2) + CENTRE)

#define WORLD_HEIGHT_CHUNKS 3

#define positiveModulo(i, n) ((i % n + n) % n)
#define wrapCoord(world, axis, coord) positiveModulo(((world)->head.axis + (coord)), AXIS_SIZE)
#define arrayCoord(world, axis, value) wrapCoord(\
    world, \
    axis, \
    (value - (world->centre.axis - RADIUS - SHIFT_ZONE))\
)

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} VECTOR;

typedef struct {
    VECTOR centre;
    struct {
        uint32_t x;
        uint32_t z;
    } head; // Top left, effective (0,0) of 2D array of chunks
    uint32_t chunks[AXIS_SIZE][AXIS_SIZE];
} World;

void worldPrint(World* world) {
    for (int x = 0; x < AXIS_SIZE; x++) {
        for (int z = 0; z < AXIS_SIZE; z++) {
            // printf(" %d ", world->chunks[wrapCoord(world, x, x)][wrapCoord(world, z, z)]);
            printf(" %d ", world->chunks[x][z]);
        }
        printf("\n");
    }
}

uint32_t worldLoadChunk(const VECTOR chunk_position) {
    // TODO
    return 1;
}

uint32_t worldUnloadChunk(const VECTOR chunk_position) {
    // TODO
    return 0;
}

__attribute__((always_inline))
inline int worldWithinLoadRadius(const World* world, const VECTOR* player_pos) {
    return absv(world->centre.x - player_pos->x) < RADIUS - 1
        && absv(world->centre.z - player_pos->z) < RADIUS - 1;
}

void worldLoadChunksX(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load x_direction chunks
    int32_t x_shift_zone = world->centre.x + ((RADIUS + SHIFT_ZONE) * x_direction);
    printf("X shift: %d\n", x_shift_zone);
    int32_t z_start;
    int32_t z_end;
    if (z_direction == -1) {
        z_start = world->centre.z - RADIUS;
        z_end = world->centre.z + RADIUS - SHIFT_ZONE;
    } else if (z_direction == 1) {
        z_start = world->centre.z - RADIUS + SHIFT_ZONE;
        z_end = world->centre.z + RADIUS;
    } else {
        z_start = world->centre.z - RADIUS;
        z_end = world->centre.z + RADIUS;
    }
    printf("Z: [%d..%d]\n", z_start, z_end);
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        uint32_t chunk = worldLoadChunk((VECTOR){
            .x = x_shift_zone,
            .y = 0, // What should this be?
            .z = z_coord
        });
        printf(
            "X Loading: %d,%d => %d,%d\n",
            x_shift_zone, z_coord,
            arrayCoord(world, x, x_shift_zone),
            arrayCoord(world, z, z_coord)
        );
        world->chunks[arrayCoord(world, x, x_shift_zone)][arrayCoord(world, z, z_coord)] = chunk;
    }
    // Unload -x_direction chunks
    x_shift_zone = world->centre.x + (RADIUS * -x_direction);
    printf("X shift: %d\n", x_shift_zone);
    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
        uint32_t unloaded_chunk = worldUnloadChunk((VECTOR){
            .x = x_shift_zone,
            .y = 0, // What should this be?
            .z = z_coord
        });
        printf(
            "X Unloading: %d,%d => %d,%d\n",
            x_shift_zone, z_coord,
            arrayCoord(world, x, x_shift_zone),
            arrayCoord(world, z, z_coord)
        );
        world->chunks[arrayCoord(world, x, x_shift_zone)][arrayCoord(world, z, z_coord)] = unloaded_chunk;
    }
}

void worldLoadChunksZ(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load z_direction chunks
    int32_t z_shift_zone = world->centre.z + ((RADIUS + SHIFT_ZONE) * z_direction);
    printf("Z shift: %d\n", z_shift_zone);
    int32_t x_start;
    int32_t x_end;
    if (x_direction == -1) {
        x_start = world->centre.x - RADIUS;
        x_end = world->centre.x + RADIUS - SHIFT_ZONE;
    } else if (x_direction == 1) {
        x_start = world->centre.x - RADIUS + SHIFT_ZONE;
        x_end = world->centre.x + RADIUS;
    } else {
        x_start = world->centre.x - RADIUS;
        x_end = world->centre.x + RADIUS;
    }
    printf("X: [%d..%d]\n", x_start, x_end);
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        uint32_t chunk = worldLoadChunk((VECTOR){
            .x = x_coord,
            .y = 0, // What should this be?
            .z = z_shift_zone
        });
        printf(
            "Z Loading: %d,%d => %d,%d\n",
            x_coord, z_shift_zone,
            arrayCoord(world, x, x_coord),
            arrayCoord(world, z, z_shift_zone)
        );
        world->chunks[arrayCoord(world, x, x_coord)][arrayCoord(world, z, z_shift_zone)] = chunk;
    }
    // Unload -x_direction chunks
    z_shift_zone = world->centre.z + (RADIUS * -z_direction);
    printf("Z shift: %d\n", z_shift_zone);
    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
        uint32_t unloaded_chunk = worldUnloadChunk((VECTOR){
            .x = x_coord,
            .y = 0, // What should this be?
            .z = z_shift_zone
        });
        printf(
            "Z Unloading: %d,%d => %d,%d\n",
            x_coord, z_shift_zone,
            arrayCoord(world, x, x_coord),
            arrayCoord(world, z, z_shift_zone)
        );
        world->chunks[arrayCoord(world, x, x_coord)][arrayCoord(world, z, z_shift_zone)] = unloaded_chunk;
    }
}

void worldLoadChunksXZ(World* world, const int8_t x_direction, const int8_t z_direction) {
    // Load (x_direction,z_direction) chunk
    int32_t x_coord = world->centre.x + ((RADIUS + SHIFT_ZONE) * x_direction);
    int32_t z_coord = world->centre.z + ((RADIUS + SHIFT_ZONE) * z_direction);
    uint32_t loaded_chunk = worldLoadChunk((VECTOR) {
        .x = x_coord,
        .y = 0, // What should this be?
        .z = z_coord
    });
    printf(
        "XZ Loading: %d,%d => %d,%d\n",
        x_coord, z_coord,
        arrayCoord(world, x, x_coord),
        arrayCoord(world, z, z_coord)
    );
    world->chunks[arrayCoord(world, x, x_coord)][arrayCoord(world, z, z_coord)] = loaded_chunk;
    // Unload (-x_direction,-z_direction) chunk
    x_coord = world->centre.x + (RADIUS * -x_direction);
    z_coord = world->centre.z + (RADIUS * -z_direction);
    uint32_t unloaded_chunk = worldUnloadChunk((VECTOR) {
        .x = x_coord,
        .y = 0, // What should this be?
        .z = z_coord
    });
    printf(
        "XZ Unloading: %d,%d => %d,%d\n",
        x_coord, z_coord,
        arrayCoord(world, x, x_coord),
        arrayCoord(world, z, z_coord)
    );
    world->chunks[arrayCoord(world, x, x_coord)][arrayCoord(world, z, z_coord)] = unloaded_chunk;
}

void worldShiftChunks(World* world, const int8_t x_direction, const int8_t z_direction) {
    uint32_t new_x = wrapCoord(world, x, x_direction);
    uint32_t new_z = wrapCoord(world, z, z_direction);
    printf("Shifted: (%d,%d) => (%d,%d)\n", world->head.x, world->head.z, new_x, new_z);
    world->head.x = new_x;
    world->head.z = new_z;
}

int relativeDirection(int32_t from, int32_t to) {
    if (to == from) {
        return 0;
    } else if (to < from) {
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
    int8_t x_direction = relativeDirection(world->centre.x, player_pos->x);
    printf("X direction: %d\n", x_direction);
    int8_t z_direction = relativeDirection(world->centre.z, player_pos->z);
    printf("Z direction: %d\n", z_direction);
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
    printf("Before shift\n");
    worldPrint(world);
    worldShiftChunks(world, x_direction, z_direction);
    // Move centre towards player position by 1 increment
    world->centre.x += x_direction;
    world->centre.z += z_direction;
    printf("World centre: %d,%d\n", world->centre.x, world->centre.z);
}

int main() {
    // In chunks
    VECTOR player_pos = {1, 0, 1};
    World _world = {
        .centre = {
            .x = 0,
            .y = 0,
            .z = 0
        },
        .chunks = {0},
    };
    World* world = &_world;
    // Default loading of chunks
    for (int x = -RADIUS; x <= RADIUS; x++) {
        for (int z = -RADIUS; z <= RADIUS; z++) {
            world->chunks[arrayCoord(world, x, x)][arrayCoord(world, z, z)] = 1;
        }
    }
    worldPrint(world);
    printf("====\n");
    worldLoadChunks(world, &player_pos);
    worldPrint(world);
    player_pos.z = 2;
    player_pos.x = 2;
    printf("====\n");
    worldLoadChunks(world, &player_pos);
    worldPrint(world);
    return 0;
}