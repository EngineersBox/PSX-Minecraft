#pragma once

#ifndef _PSXMC__GAME_WORLD_REGION__REGION_H_
#define _PSXMC__GAME_WORLD_REGION__REGION_H_

#include "../../../util/inttypes.h"

#define REGION_RADIUS 2
#define REGION_WIDTH ((REGION_RADIUS * 2) + 1)
#define REGION_CHUNKS_COUNT (REGION_WIDTH * REGION_WIDTH * REGION_WIDTH)

typedef struct RegionChunk {
    u8 generated_sub_chunks;

} RegionChunk;

typedef struct Region {
    u8 generated_chunks;
    RegionChunk chunks[REGION_CHUNKS_COUNT];
} Region;

#endif // _PSXMC__GAME_WORLD_REGION__REGION_H_
