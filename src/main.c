#include <display.h>
#include <stdint.h>
#include <stdio.h>
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
        { ONE * 3 / 4, 0, 0 }, /* Red   */
        { ONE * 3 / 4, 0, 0 }, /* Green */
        { ONE * 3 / 4, 0, 0 } /* Blue  */
    }
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX light_mtx = {
    /* X,  Y,  Z */
    .m = {
        { -fixedDiv(ONE, 2), -fixedDiv(ONE, 2), -fixedDiv(ONE, 2) },
        { 0, 0, 0 },
        { 0, 0, 0 }
    }
};

const SVECTOR CUBE_VERTICES[8] = {
    {-25, -25, -25, 0}, // 0
    {25, -25, -25, 0}, // 1
    {-25, 25, -25, 0}, // 2
    {25, 25, -25, 0}, // 3
    {25, -25, 25, 0}, // 4
    {-25, -25, 25, 0}, // 5
    {25, 25, 25, 0}, // 6
    {-25, 25, 25, 0} // 7
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
        .position = {0, ONE * -300, ONE * -300},
        .rotation = {0, 0, 0},
        .mode = 0
    };
    Transforms transforms = {
        .translation_rotation = {0},
        .translation_position = {0, 0, 200},
        .geometry_mtx = {},
        .lighting_mtx = light_mtx
    };
    init();
    World world;
    world.centre.vx = 0;
    world.centre.vy = 1;
    world.centre.vz = 0;
    worldInit(&world);
    // Chunk chunk;
    // chunk.position = (VECTOR) {0, 0, 0};
    // chunkInit(&chunk);
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

typedef struct {
    SMD smd;
    cvector(SVECTOR) vec1;
    cvector(SVECTOR) vec2;
} TestMinRep;

void __destruct(void *elem) {
}

void initRep(TestMinRep *rep) {
    rep->vec1 = NULL;
    rep->vec2 = NULL;
    cvector_init(rep->vec1, 1, __destruct);
    cvector_init(rep->vec2, 1, __destruct);
}

int _main() {
    init();
    TestMinRep obj;
    initRep(&obj);
    uint32_t i1 = 0;
    uint32_t i2 = 0;
    int should_break = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            printf("VERTEX %d: %p\n", j, obj.vec1);
            cvector_metadata_t *meta = cvector_vec_to_base(obj.vec1);
            printf("  [BEFORE] Size: %d, Cap: %d, Dest: %p\n", meta->size, meta->capacity, meta->elem_destructor);
            cvector_metadata_t *meta2 = cvector_vec_to_base(obj.vec2);
            printf("  [Normal: %p] [BEFORE] Size: %d, Cap: %d, Dest: %p\n", obj.vec2, meta2->size, meta2->capacity,
                   meta2->elem_destructor);
            cvector_push_back(obj.vec1, (SVECTOR){});
            meta = cvector_vec_to_base(obj.vec1);
            printf("  [AFTER] Size: %d, Cap: %d, Dest: %p\n", meta->size, meta->capacity, meta->elem_destructor);
            meta2 = cvector_vec_to_base(obj.vec2);
            printf("  [Normal: %p] [AFTER] Size: %d, Cap: %d, Dest: %p\n", obj.vec2, meta2->size, meta2->capacity,
                   meta2->elem_destructor);
            SVECTOR *curr = &obj.vec1[i1];
            curr->vx = i;
            curr->vy = j;
            curr->vz = i1;
            i1++;
            if (i == 2 && j == 1) {
                should_break = 1;
                break;
            }
        }
        if (should_break) {
            break;
        }
        printf("NORMAL: %p\n", obj.vec2);
        cvector_metadata_t *meta2 = cvector_vec_to_base(obj.vec2);
        printf("  [BEFORE] Size: %d, Cap: %d, Dest: %p\n", meta2->size, meta2->capacity, meta2->elem_destructor);
        cvector_push_back(obj.vec2, (SVECTOR){});
        meta2 = cvector_vec_to_base(obj.vec2);
        printf("  [AFTER] Size: %d, Cap: %d, Dest: %p\n", meta2->size, meta2->capacity, meta2->elem_destructor);
        SVECTOR *curr1 = &obj.vec2[i2];
        curr1->vx = i;
        curr1->vx = i2;
        curr1->vz = rand();
        i2++;
    }
    while (1) {
        display(&dctx);
    }
    return 0;
}
