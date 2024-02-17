//#include <stdio.h>
//#include <stdint.h>
//
//#define WORLD_CHUNKS_HEIGHT 1
//#define WORLD_HEIGHT (CHUNK_SIZE * WORLD_CHUNKS_HEIGHT)
//
//// Must be positive
//#define LOADED_CHUNKS_RADIUS 1
//#define SHIFT_ZONE 1
//#define CENTER 1
//#define WORLD_CHUNKS_RADIUS (LOADED_CHUNKS_RADIUS + CENTER)
//#if LOADED_CHUNKS_RADIUS < 1
//#define AXIS_CHUNKS (CENTER + SHIFT_ZONE)
//#else
//#define AXIS_CHUNKS (((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * 2) + CENTER)
//#endif
//
///**
// * @brief Compare two numbers and return an integer indicating larger value
// * @param a - first number
// * @param b - second number
// * @return 0 if a == b, 1 if b > a, -1 if b < a
// */
//#define cmp(a, b) (((b) > (a)) - ((b) < (a)))
//
///**
// * @brief Retrieve the sign (1 for positive, 0 for negative) of a number
// * @param v - number to apply to
// * @return 1 if v >= 0, otherwise 0
// */
//#define sign(v) cmp(0, v)
//
///**
// * @brief Absolute value of an number
// * @param v - number to apply to
// * @return -v if v < 0, otherwise v
// */
//#define absv(v) ((v) * sign(v))
//
//int32_t positiveModulo(int32_t i, int32_t n) {
//    return (((i % n) + n) % n);
//}
//
//// #define positiveModulo(i, n) ((((i) % (n)) + (n)) % (n))
//
//#define wrapCoord(world, axis, coord) positiveModulo(((world)->head.axis + (coord)), AXIS_CHUNKS)
//
//#define arrayCoord(world, axis, value) wrapCoord(\
//    world, \
//    axis, \
//    ((value) - ((world)->centre.axis - LOADED_CHUNKS_RADIUS - SHIFT_ZONE))\
//)
//
//typedef struct {
//    int32_t vx;
//    int32_t vy;
//    int32_t vz;
//    int32_t code;
//} VECTOR;
//
//typedef struct World {
//    VECTOR centre;
//    struct {
//        uint32_t vx;
//        uint32_t vz;
//    } head; // Top left, effective (0,0) of 2D array of chunks
//    // X, Z, Y
//    uint32_t chunks[AXIS_CHUNKS][AXIS_CHUNKS];
//} World;
//
//int32_t worldLoadChunk(World* world, const VECTOR chunk_position) {
//    return 1;
//}
//
//void worldLoadChunksX(World* world, const int8_t x_direction, const int8_t z_direction) {
//    // Load x_direction chunks
//    int32_t x_shift_zone = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
//    int32_t z_start = world->centre.vz - LOADED_CHUNKS_RADIUS;
//    int32_t z_end = world->centre.vz + LOADED_CHUNKS_RADIUS;
//    if (z_direction == -1) {
//        z_end -= SHIFT_ZONE;
//    } else if (z_direction == 1) {
//        z_start += SHIFT_ZONE;
//    }
//    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
//        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//            int32_t chunk = worldLoadChunk(world, (VECTOR){
//                                              .vx = x_shift_zone,
//                                              .vy = y,
//                                              .vz = z_coord
//                                          });
//            world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)] = chunk;
//        }
//    }
//    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
//        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//            world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)]; // Gen mesh
//        }
//    }
//    // Unload -x_direction chunks
//    x_shift_zone = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
//    for (int z_coord = z_start; z_coord <= z_end; z_coord++) {
//        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//            world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_shift_zone)] = 0;
//        }
//    }
//}
//
//void worldLoadChunksZ(World* world, const int8_t x_direction, const int8_t z_direction) {
//    // Load z_direction chunks
//    int32_t z_shift_zone = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
//    int32_t x_start = world->centre.vx - LOADED_CHUNKS_RADIUS;
//    int32_t x_end = world->centre.vx + LOADED_CHUNKS_RADIUS;
//    /* Can be simplified to:
//     * int32_t x_start = world->centre.vx - LOADED_CHUNKS_RADIUS + ((x_direction == -1) * SHIFT_ZONE);
//     * int32_t x_end = world->centre.vz + LOADED_CHUNKS_RADIUS - ((x_direction == 1) * SHIFT_ZONE);
//     */
//    if (x_direction == -1) {
//        x_end -= SHIFT_ZONE;
//    } else if (x_direction == 1) {
//        x_start += SHIFT_ZONE;
//    }
//    printf("Z shift: %d\n", z_shift_zone);
//    printf("X [Start: %d] [End: %d]\n", x_start, x_end);
//    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
//        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//            int32_t chunk = worldLoadChunk(world, (VECTOR){
//                                              .vx = x_coord,
//                                              .vy = y,
//                                              .vz = z_shift_zone
//                                          });
//            printf(
//                "Loading (%d,%d,%d) -> [%d][%d]\n",
//                x_coord, y, z_shift_zone,
//                arrayCoord(world, vz, z_shift_zone), arrayCoord(world, vx, x_coord)
//            );
//            printf("VZ: %d\n", arrayCoord(world, vz, z_shift_zone));
//            world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)] = chunk;
//        }
//    }
//    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
//        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//            world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)]; // Gen mesh
//        }
//    }
//    // Unload -z_direction chunks
//    z_shift_zone = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
//    for (int x_coord = x_start; x_coord <= x_end; x_coord++) {
//        for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//            world->chunks[arrayCoord(world, vz, z_shift_zone)][arrayCoord(world, vx, x_coord)] = 0;
//        }
//    }
//}
//
//void worldLoadChunksXZ(World* world, const int8_t x_direction, const int8_t z_direction) {
//    // Load (x_direction,z_direction) chunk
//    int32_t x_coord = world->centre.vx + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * x_direction);
//    int32_t z_coord = world->centre.vz + ((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * z_direction);
//    for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//        int32_t loaded_chunk = worldLoadChunk(world, (VECTOR){
//                                                 .vx = x_coord,
//                                                 .vy = y,
//                                                 .vz = z_coord
//                                             });
//        world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)] = loaded_chunk;
//    }
//    for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//        world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)]; // Gen mesh
//    }
//    // Unload (-x_direction,-z_direction) chunk
//    x_coord = world->centre.vx + (LOADED_CHUNKS_RADIUS * -x_direction);
//    z_coord = world->centre.vz + (LOADED_CHUNKS_RADIUS * -z_direction);
//    for (int y = 0; y < WORLD_CHUNKS_HEIGHT; y++) {
//        world->chunks[arrayCoord(world, vz, z_coord)][arrayCoord(world, vx, x_coord)] = 0;
//    }
//}
//
//void worldShiftChunks(World* world, const int8_t x_direction, const int8_t z_direction) {
//    world->head.vx = wrapCoord(world, vx, x_direction);
//    world->head.vz = wrapCoord(world, vz, z_direction);
//}
//
//__attribute__((always_inline))
//inline int worldWithinLoadRadius(const World* world, const VECTOR* player_chunk_pos) {
//    return absv(world->centre.vx - player_chunk_pos->vx) < LOADED_CHUNKS_RADIUS - 1
//// #if LOADED_CHUNKS_RADIUS == 1
////            < LOADED_CHUNKS_RADIUS
//// #else
////            < LOADED_CHUNKS_RADIUS - 1
//// #endif
//           && absv(world->centre.vz - player_chunk_pos->vz) < LOADED_CHUNKS_RADIUS - 1
//// #if LOADED_CHUNKS_RADIUS == 1
////            < LOADED_CHUNKS_RADIUS
//// #else
////            < LOADED_CHUNKS_RADIUS - 1
//// #endif
//        ;
//}
//
//// BUG: Fix player movement loading chunks in opposite direction
//void worldLoadChunks(World* world, const VECTOR* player_chunk_pos) {
//    // Check if we need to load
//    if (worldWithinLoadRadius(world, player_chunk_pos)) {
//        return;
//    }
//    // Calculate direction shifts
//    const int8_t x_direction = cmp(world->centre.vx, player_chunk_pos->vx);
//    const int8_t z_direction = cmp(world->centre.vz, player_chunk_pos->vz);
//    printf("x_dir: %d, z_dir: %d\n", x_direction, z_direction);
//    // Load chunks
//    if (x_direction != 0) {
//        worldLoadChunksX(world, x_direction, z_direction);
//    }
//    if (z_direction != 0) {
//        worldLoadChunksZ(world, x_direction, z_direction);
//    }
//    if (x_direction != 0 && z_direction != 0) {
//        worldLoadChunksXZ(world, x_direction, z_direction);
//    }
//    // Shift chunks into centre of arrays
//    worldShiftChunks(world, x_direction, z_direction);
//    // Move centre towards player position by 1 increment
//    world->centre.vx += x_direction;
//    world->centre.vz += z_direction;
//}
//
//void worldPrint(World* world) {
//    for (int z = 0; z < AXIS_CHUNKS; z++) {
//        for (int x = 0; x < AXIS_CHUNKS; x++) {
//            printf("%d ", world->chunks[z][x] != 0);
//        }
//        printf("\n");
//    }
//}
//
//void worldUpdate(World* world, const VECTOR* player_pos) {
//    // TODO: These are temp testing static vars, remove them later
//    static int32_t prevx = 0;
//    static int32_t prevy = 0;
//    static int32_t prevz = 0;
//    if (player_pos->vx != prevx
//        || player_pos->vy != prevy
//        || player_pos->vz != prevz) {
//        prevx = player_pos->vx;
//        prevy = player_pos->vy;
//        prevz = player_pos->vz;
//        worldLoadChunks(world, player_pos);
//        printf(
//            "[WORLD] Head { x: %d, z: %d } Centre { x: %d, z: %d}\n",
//            world->head.vx, world->head.vz,
//            world->centre.vx, world->centre.vz
//        );
//        worldPrint(world);
//    }
//}
//
//void initChunks(World* world) {
//    for (int z = 0; z < AXIS_CHUNKS; z++) {
//        for (int x = 0; x < AXIS_CHUNKS; x++) {
//            world->chunks[z][x] = x > 0 && x < AXIS_CHUNKS - 1 && z > 0 && z < AXIS_CHUNKS - 1;
//        }
//    }
//}
//
//int32_t posMod(int32_t i, int32_t n) {
//    return (((i % n) + n) % n);
//}
//
//int main() {
//    World world = (World) {
//        .centre = (VECTOR) {
//            .vx = 0,
//            .vy = 0,
//            .vz = 0,
//        },
//        .head = {
//            .vx = 0,
//            .vz = 0,
//        }
//    };
//    printf("AXIS_CHUNKS: %d\n", AXIS_CHUNKS);
//    VECTOR player_pos = (VECTOR) {
//        .vx = 0,
//        .vy = 0,
//        .vz = -1
//    };
//    initChunks(&world);
//    worldPrint(&world);
//    worldUpdate(&world, &player_pos);
//    player_pos.vz = -2;
//    printf("====\n");
//    worldPrint(&world);
//    worldUpdate(&world, &player_pos);
//    // printf("%d\n", positiveModulo(0 + player_pos.vz, AXIS_CHUNKS));
//    // printf("2: %d\n", positiveModulo(((&world)->head.vz + player_pos.vz), AXIS_CHUNKS));
//    // printf("3: %d\n", posMod((&world)->head.vz + player_pos.vz, AXIS_CHUNKS));
//    // printf("head z: %d -> %d\n", 0, wrapCoord(&world, vz, player_pos.vz));
//    // printf("%d -> %d\n", -2, arrayCoord(&world, vz, -2));
//
//    return 0;
//}
//