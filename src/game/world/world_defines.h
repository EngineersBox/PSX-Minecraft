#pragma once

#ifndef _PSXMC__GAME_WORLD__WORLD_DEFINES_H_
#define _PSXMC__GAME_WORLD__WORLD_DEFINES_H_

// ONE * 0.01 = 40.96
#define WEATHER_STRENGTH_INCREMENT 41

#define WORLD_CHUNKS_HEIGHT 1
#define WORLD_HEIGHT (CHUNK_SIZE * WORLD_CHUNKS_HEIGHT)

// 20 min * 60 sec * 20 ticks
#define WORLD_TIME_CYCLE 24000
#define WORLD_TIME_NOON 6000
#define WORLD_TIME_DUSK 12000
#define WORLD_TIME_MIDNIGHT 18000
#define WORLD_TIME_DAWN 0

// TODO: Make these properties configurable as externs
//       to be accessible via some options interface
#ifndef LOADED_CHUNKS_RADIUS
// Must be positive
#define LOADED_CHUNKS_RADIUS 1
#endif
#define SHIFT_ZONE 1
#define CENTER 1
#define WORLD_CHUNKS_RADIUS (LOADED_CHUNKS_RADIUS + CENTER)
#if LOADED_CHUNKS_RADIUS < 1
#define AXIS_CHUNKS (CENTER + SHIFT_ZONE)
#else
#define AXIS_CHUNKS (((LOADED_CHUNKS_RADIUS + SHIFT_ZONE) * 2) + CENTER)
#endif
#define WORLD_CHUNKS_COUNT (AXIS_CHUNKS * AXIS_CHUNKS * WORLD_CHUNKS_HEIGHT)

#endif // _PSXMC__GAME_WORLD__WORLD_DEFINES_H_