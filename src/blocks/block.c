#include "block.h"

void blockRender(Block* block, DisplayContext* ctx, Transforms* transforms) {

}

Cube _cube = (Cube) {
    .position = {0},
    .rotation = {0},
    .texture_face_attrib = {
        {3 * 16, 0, 16, 16, {0}}, // -Z FRONT
        {3 * 16, 0, 16, 16, {0}}, // +Z BACK
        {0 * 16, 0, 16, 16, {0, 155, 0, 1}}, // -Y TOP
        {2 * 16, 0, 16, 16, {0}}, // +Y BOTTOM
        {3 * 16, 0, 16, 16, {0}}, // -X LEFT
        {3 * 16, 0, 16, 16, {0}}  // +X RIGHT
    },
    .texture = 0,
    .vertices = (SVECTOR[8]) {
        {-BLOCK_SIZE, -BLOCK_SIZE, -BLOCK_SIZE, 0}, // 0
        {BLOCK_SIZE, -BLOCK_SIZE, -BLOCK_SIZE, 0}, // 1
        {-BLOCK_SIZE, BLOCK_SIZE, -BLOCK_SIZE, 0}, // 2
        {BLOCK_SIZE, BLOCK_SIZE, -BLOCK_SIZE, 0}, // 3
        {BLOCK_SIZE, -BLOCK_SIZE, BLOCK_SIZE, 0}, // 4
        {-BLOCK_SIZE, -BLOCK_SIZE, BLOCK_SIZE, 0}, // 5
        {BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 0}, // 6
        {-BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 0} // 7
    }
};


const Block BLOCKS[BLOCK_COUNT] = {
    (Block) {
        .id = 0,
        .cube = NULL,
        .name = "air",
        .type = AIR
    },
    (Block) {
        .id = 1,
        .cube = &_cube,
        .name = "grass",
        .type = SOLID
    }
};