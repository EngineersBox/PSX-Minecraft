/******************************************************************************

Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
#include <stdint.h>

#define BLOCK_SIZE 50
#define BLOCK_FACES 6
#define BLOCK_TEXTURE_SIZE 16
#define BLOCK_COUNT 256

typedef uint8_t BlockID;

typedef enum {
    BLOCKTYPE_EMPTY = 0,
    BLOCKTYPE_SOLID,
    BLOCKTYPE_STAIR,
    BLOCKTYPE_SLAB,
    BLOCKTYPE_CROSS,
    BLOCKTYPE_HASH
} BlockType;

typedef enum {
    ORIENTATION_POS_X = 0,
    ORIENTATION_NEG_X,
    ORIENTATION_POS_Y,
    ORIENTATION_NEG_Y,
    ORIENTATION_POS_Z,
    ORIENTATION_NEG_Z
} Orientation;

typedef struct {
    BlockID id;
    BlockType type;
    Orientation orientation;
    int16_t face_attributes[BLOCK_FACES];
    char* name;
} Block;

typedef struct {
    Block block;
    int8_t stone_type;
    char* formatted;
} Stone;

int main() {
    Stone stone = (Stone) {
        .block = (Block) {
            .id = 1,
            .type = BLOCKTYPE_SOLID,
            .orientation = ORIENTATION_POS_X,
            .face_attributes = { 1, 2, 3, 4, 5, 6  },
            .name = "stone"
        },
        .stone_type = 3,
        .formatted = "test"
    };
    Block* block = (Block*) &stone;
    printf(
        "id: %d, type: %d, orientation: %d, face attributes: [%d,%d,%d,%d,%d,%d], name: %s\n",
        block->id,
        block->type,
        block->orientation,
        block->face_attributes[0],
        block->face_attributes[1],
        block->face_attributes[2],
        block->face_attributes[3],
        block->face_attributes[4],
        block->face_attributes[5],
        block->name
    );
    return 0;
}
