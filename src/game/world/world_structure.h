#pragma once

#ifndef _PSXMC__GAME_WORLD__WORLD_STRUCTURE_H_
#define _PSXMC__GAME_WORLD__WORLD_STRUCTURE_H_

#include <stdbool.h>
#include <psxgte.h>

#include "generation/chunk_provider.h"
#include "../../lighting/lightmap.h"
#include "../../util/inttypes.h"
#include "world_defines.h"
#include "chunk/heightmap.h"

typedef struct Weather {
    // Value in range [0, ONE] aka [0, 4096]
    fixedi16 rain_strength;
    // Value in range [0, ONE] aka [0, 4096]
    fixedi16 storm_strength;
    u32 rain_time_ticks;
    u32 storm_time_ticks;
    bool raining: 1;
    bool storming: 1;
    u8 _pad: 6;
} Weather;

typedef struct World {
    VECTOR centre;
    VECTOR centre_next;
    struct {
        u32 vx;
        u32 vz;
    } head; // Top left, effective (0,0) of 2D array of chunks
    LightLevel internal_light_level;
    u16 time_ticks;
    u32 day_count;
    fixedi32 celestial_angle;
    Weather weather;
    IChunkProvider chunk_provider;
    Heightmap heightmap;
    // X, Z, Y
    Chunk* chunks[AXIS_CHUNKS][AXIS_CHUNKS][WORLD_CHUNKS_HEIGHT];
} World;

extern World* world;

#endif // _PSXMC__GAME_WORLD__WORLD_STRUCTURE_H_
