#pragma once

#ifndef _PSXMC__LIGHTING__LIGHTMAP_H_
#define _PSXMC__LIGHTING__LIGHTMAP_H_

#include <psxgte.h>

#include "../util/inttypes.h"
#include "../math/math_utils.h"
#include "../game/world/chunk/chunk_structure.h"

#define LIGHT_BLOCK_MASK 0b11110000
#define LIGHT_SKY_MASK 0b00001111

typedef u8 LightMap[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

typedef enum LightType {
    LIGHT_TYPE_SKY = 0,
    LIGHT_TYPE_BLOCK
} LightType;

void lightMapSetValue(LightMap lightmap, const VECTOR position, u8 light_value, const LightType light_type);
u8 lightMapGetType(const LightMap lightmap, const VECTOR position, const LightType light_type);
u8 lightMapGetValue(const LightMap lightmap, const VECTOR position);

u8 lightLevelToOverlayColour(const u8 light_value);
u8 lightLevelApplicable(const u8 light_value);


#endif // _PSXMC__LIGHTING__LIGHTMAP_H_
