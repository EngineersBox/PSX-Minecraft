#pragma once

#ifndef _PSXMC__GAME_WORLD_CHUNK__CHUNK_VISIBILITY_H_
#define _PSXMC__GAME_WORLD_CHUNK__CHUNK_VISIBILITY_H_

#include "../../../util/inttypes.h"
#include "../../../util/preprocessor.h"
#include "../../../math/math_utils.h"
#include "chunk_defines.h"

#if defined(CHUNK_SIZE) && CHUNK_SIZE > 0 && CHUNK_SIZE <= 32 && _isPowerOf2(CHUNK_SIZE)
    #define visibilityType(size, name) typedef GLUE(u, size) name
    visibilityType(CHUNK_SIZE, ChunkVisibility);
#undef visibilityType
#else
#error CHUNK_SIZE must be in the interval (0, 32] and be a power of 2
#endif

u8 chunkVisibilityGetBit(const ChunkVisibility vis, const u8 a, const u8 b);
void chunkVisibilitySetBit(ChunkVisibility* vis, const u8 a, const u8 b);

#endif // _PSXMC__GAME_WORLD_CHUNK__CHUNK_VISIBILITY_H_
