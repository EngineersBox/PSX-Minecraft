#pragma once

#ifndef _PSXMC__GAME_WORLD_CHUNK__CHUNK_VISIBILITY_H_
#define _PSXMC__GAME_WORLD_CHUNK__CHUNK_VISIBILITY_H_

#include "../../../util/inttypes.h"
#include "../../../util/preprocessor.h"
#include "../../../math/math_utils.h"
#include "chunk_defines.h"

// Direction bitmask
// LRFBUD   Val   Idx <- AKA log2(Val)
// 000001 ->  1 -> 0 = D
// 000010 ->  2 -> 1 = U
// 000100 ->  4 -> 2 = B
// 001000 ->  8 -> 3 = F
// 010000 -> 16 -> 4 = R
// 100000 -> 32 -> 5 = L
//
//    Bitmask   Val   Idx   Num
// LR 110000 -> 48 -> 14  -+
// LF 101000 -> 40 -> 13   |
// LB 100100 -> 36 -> 12   | 5
// LU 100010 -> 34 -> 11   |
// LD 100001 -> 33 -> 10  -+
// RF 011000 -> 24 ->  9  -+
// RB 010100 -> 20 ->  8   | 4
// RU 010010 -> 18 ->  7   |
// RD 010001 -> 17 ->  6  -+
// FB 001100 -> 12 ->  5  -+
// FU 001010 -> 10 ->  4   | 3
// FD 001001 ->  9 ->  3  -+
// BU 000110 ->  6 ->  2  -+ 2
// BD 000101 ->  5 ->  1  -+
// UD 000011 ->  3 ->  0   ] 1
// 
// The combined bitmask is just a monotonic
// series of sequential integers. The offset
// to any of the sections of the bitmask sections
// is ((n - 1) * n) >> 1, where n is the max(a,b)
// of the two directions in question. We then
// just add the min(a,b) of of these directions
// onto that value to get the bit index into
// the bitset.
// 
// I.e. for directions L and B with directional
// bitmasks of 100000 and 000100 repectively, this
// is:
// max(5, 2) => 5
// min(5, 2) => 2
// ((5 - 1) * 5) >> 2 => 10
// therefore bit index = 10 + 2 => 12

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
