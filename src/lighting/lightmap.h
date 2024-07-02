#pragma once

#ifndef _PSX_MINECRAFT__LIGHTING__LIGHTMAP_H_
#define _PSX_MINECRAFT__LIGHTING__LIGHTMAP_H_

#include <psxgte.h>

#include "../util/inttypes.h"
#include "../game/world/chunk/chunk_structure.h"

typedef u8 LightMap[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

void lightMapSetBlock(LightMap lightmap, const VECTOR position, u8 light_value);
u8 lightMapGetBlock(const LightMap lightmap, const VECTOR position);

#endif // _PSX_MINECRAFT__LIGHTING__LIGHTMAP_H_
