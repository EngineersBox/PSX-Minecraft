#pragma once

#ifndef PSX_MINECRAFT_BLOCKS_H
#define PSX_MINECRAFT_BLOCKS_H

#include "block_id.h"
#include "block.h"
#include "block_air.h"
#include "block_stone.h"
#include "block_dirt.h"
#include "block_grass.h"

#define BLOCK_COUNT 256

extern uint8_t _last_block_index;
extern Block BLOCKS[BLOCK_COUNT];

#define blockInitialise(blockDef) ({ \
    if (_last_block_index >= BLOCK_COUNT) { \
        printf("[ERROR] Maximum blocks declared\n"); \
        abort(); \
    } \
    BLOCKS[_last_block_index++] = blockDef; \
})

void blocksInitialiseBuiltin();

#endif // PSX_MINECRAFT_BLOCKS_H
