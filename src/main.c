#include <display.h>
#include <stdint.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#include "resources/assets.h"
#include "core/input.h"
#include "core/camera.h"
#include "primitive/cube.h"
#include "world/world.h"
#include "util/math_utils.h"

DisplayContext dctx = {
    .active = 0,
    .db = {},
    .primitive = NULL
};
Input input = {};

// Light color matrix
// Each column represents the color matrix of each light source and is
// used as material color when using gte_ncs() or multiplied by a
// source color when using gte_nccs(). 4096 is 1.0 in this matrix
// A column of zeroes effectively disables the light source.
MATRIX color_mtx = {
    .m = {
        {ONE * 3 / 4, 0, 0}, /* Red   */
        {ONE * 3 / 4, 0, 0}, /* Green */
        {ONE * 3 / 4, 0, 0} /* Blue  */
    }
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX light_mtx = {
    /* X,  Y,  Z */
    .m = {
        {-FIXED_1_2, -FIXED_1_2, FIXED_1_2},
        {0, 0, 0},
        {0, 0, 0}
    }
};

// Reference texture data
extern const uint32_t tim_texture[];

void init() {
    initDisplay(&dctx);
    initInput(&input);
    /* Set light ambient color and light color matrix */
    gte_SetBackColor(63, 63, 63);
    gte_SetColorMatrix(&color_mtx);
    // Load font and open a text stream
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 100);
    // Unpack LZP archive and load assets
    assetsLoad();
}

// static int random[128] = {
//     1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1,
//     1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
//     0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1,
//     0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1
// };
//
// void initTestBlocks(Chunk *chunk) {
//     // for (int x = 0; x < CHUNK_SIZE; x++) {
//     //     for (int z = 0; z < CHUNK_SIZE; z++) {
//     //         for (int y = 0; y < CHUNK_SIZE; y++) {
//     //             chunk->blocks[chunkBlockIndex(x, y, z)] = (BlockID) STONE;
//     //         }
//     //     }
//     // }
//     for (int x = 0; x < CHUNK_SIZE; x++) {
//         for (int z = 0; z < CHUNK_SIZE; z++) {
//             for (int y = 0; y < 3; y++) {
//                 if (y == 0) {
//                     chunk->blocks[chunkBlockIndex(x, y, z)] = (BlockID) STONE;
//                 } else if (y == 1) {
//                     chunk->blocks[chunkBlockIndex(x, y, z)] = random[x + (z * CHUNK_SIZE)] == 1
//                                                                   ? (BlockID) DIRT
//                                                                   : (BlockID) AIR;
//                 } else if (y == 2) {
//                     chunk->blocks[chunkBlockIndex(x, y, z)] = random[64 + x + (z * CHUNK_SIZE)] == 1
//                                                                   ? (BlockID) GRASS
//                                                                   : (BlockID) AIR;
//                 }
//             }
//         }
//     }
// }

void cameraReset(Camera* camera) {
    camera->position = (VECTOR){0, ONE * 0, 0};
    camera->rotation = (VECTOR){0, 0, 0};
    camera->mode = 0;
}

int main() {
    VECTOR look_pos = {0};
    Camera camera = {
        .position = { 0, ONE * - 200, ONE * -200 },
        // .position = {ONE * -632, ONE * -117, ONE * 233},
        .rotation = {ONE * 248, ONE * -1592, 0},
        .mode = 0
    };
    Transforms transforms = {
        .translation_rotation = {0},
        .translation_position = {0, 0, 200},
        .geometry_mtx = {},
        .lighting_mtx = light_mtx
    };
    init();
    World world = {
        .head = {
            .vx = 0,
            .vz = 0
        },
        .centre = {
            .vx = 0,
            .vy = 0,
            .vz = 0
        }
    };
    worldInit(&world);
    // Chunk chunk;
    // chunk.position = (VECTOR) {0, 1, 0};
    // chunkInit(&chunk);
    // chunkGenerateMesh(&chunk);
    // Chunk chunk1;
    // chunk1.position = (VECTOR) {1, 1, 0};
    // initTestBlocks(&chunk1);
    // chunkInit(&chunk1);
    while (1) {
        // Set pad pointer to buffer data
        camera.mode = 0;
        cameraUpdate(&camera, &input, &transforms, &look_pos);
        // Set rotation and translation matrix
        gte_SetRotMatrix(&transforms.geometry_mtx);
        gte_SetTransMatrix(&transforms.geometry_mtx);
        // Draw the world
        worldRender(&world, &dctx, &transforms);
        // Draw the chunk
        // chunkRender(&chunk, &dctx, &transforms);
        // chunkRender(&chunk1, &dctx, &transforms);
        // Draw cubes
        // cubeRender(&cube, &dctx, &transforms);
        // cubeRender(&cube1, &dctx, &transforms);
        // Clear window constraints
        displayClearConstraints(&dctx);
        // Flush font to screen
        FntFlush(-1);
        // Swap buffers and draw the primitives
        display(&dctx);
    }
    // chunkDestroy(&chunk);
    worldDestroy(&world);
    assetsFree();
    return 0;
}
