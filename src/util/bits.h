#pragma once

#ifndef PSXMC_BITS_H
#define PSXMC_BITS_H

#include "inttypes.h"

u8 trailing_zeros(u32 value);
u8 trailing_ones(u32 value);

#define bitmapGetBit(_bitmap, update_type) (((_bitmap) >> (update_type)) & 0b1)
#define bitmapSetBit(_bitmap, update_type) ((_bitmap) |= 1 << (update_type))
#define bitmapUnsetBit(_bitmap, update_type) ((_bitmap) &= ~(1 << (update_type)))

#endif // PSXMC_BITS_H
