#pragma once

#ifndef _PSXMC__LIGHTING__LIGHTMAP_H_
#define _PSXMC__LIGHTING__LIGHTMAP_H_

#include <psxgte.h>

#include "../util/inttypes.h"
#include "../math/math_utils.h"
#include "../game/world/chunk/chunk_defines.h"

#define LIGHT_BLOCK_MASK 0b11110000
#define LIGHT_SKY_MASK 0b00001111
#define createLightLevel(block, sky) ((((block) << 4) & LIGHT_BLOCK_MASK) | ((sky) & LIGHT_SKY_MASK))
// ((4096 << 12) / (16 << 12))
#define SCALE_PER_LIGHT_LEVEL 256

typedef u8 LightLevel;
typedef LightLevel LightMap[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

typedef enum LightType {
    LIGHT_TYPE_SKY = 0,
    LIGHT_TYPE_BLOCK
} LightType;

void lightMapSetValue(LightMap lightmap,
                      const VECTOR position,
                      const LightLevel light_value,
                      const LightType light_type);
LightLevel lightMapGetType(const LightMap lightmap, const VECTOR position, const LightType light_type);
LightLevel lightMapGetValue(const LightMap lightmap, const VECTOR position);

LightLevel lightLevelToOverlayColour(const LightLevel light_value);
LightLevel lightLevelApplicable(const LightLevel internal_light_level, const LightLevel light_value);


#endif // _PSXMC__LIGHTING__LIGHTMAP_H_
