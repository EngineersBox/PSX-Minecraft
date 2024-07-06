#pragma once

#ifndef _PSX_MINECRAFT__LIGHTING__LIGHTMAP_H_
#define _PSX_MINECRAFT__LIGHTING__LIGHTMAP_H_

#include <psxgte.h>

#include "../util/inttypes.h"
#include "../game/world/chunk/chunk_structure.h"

typedef u8 LightMap[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

typedef enum LightType {
    LIGHT_TYPE_SKY = 0,
    LIGHT_TYPE_BLOCK
} LightType;

void lightMapSetValue(LightMap lightmap, const VECTOR position, u8 light_value, const LightType light_type);
u8 lightMapGetValue(const LightMap lightmap, const VECTOR position, const LightType light_type);

#endif // _PSX_MINECRAFT__LIGHTING__LIGHTMAP_H_
