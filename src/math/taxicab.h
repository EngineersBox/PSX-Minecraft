#pragma once

#ifndef _MATH__TAXICAB_H_
#define _MATH__TAXICAB_H_

#include <stdbool.h>

#include "fixed_point.h"

/**
 * Q12 fixed point taxicab radians
 * Range: 0 - 16384 (4 << FIXED_POINT_SHIFT)
 */
typedef fixedi32 TRad;

#define TRAD_MIN 0
#define TRAD_MAX (4 << FIXED_POINT_SHIFT)

// (10 / 90) * TRAD_90_DEG = 455.11111...
#define TRAD_10_DEG 455
// (20 / 90) * TRAD_90_DEG = 910.22222...
#define TRAD_20_DEG 910
// (30 / 90) * TRAD_90_DEG = 1365.33333...
#define TRAD_30_DEG 1365
// (40 / 90) * TRAD_90_DEG = 1820.44444...
#define TRAD_40_DEG 1820
// (45 / 90) * TRAD_90_DEG = 2048
#define TRAD_45_DEG 2048
// (50 / 90) * TRAD_90_DEG = 2275.55555...
#define TRAD_50_DEG 2276
// (60 / 90) * TRAD_90_DEG = 2730.66666...
#define TRAD_60_DEG 2731
// (70 / 90) * TRAD_90_DEG = 3185.77777...
#define TRAD_70_DEG 3186
// (80 / 90) * TRAD_90_DEG = 3640.88888...
#define TRAD_80_DEG 3641
// (ONE << 12) / ONE = 4096
#define TRAD_90_DEG ONE

TRad tcabAngle(const fixedi32 x, const fixedi32 y);

bool tcabAngleInRange(TRad ref,
                      const TRad angle,
                      TRad query);

bool tcabAngleRangeOverlap(TRad ref,
                           const TRad angle,
                           TRad range_start,
                           TRad range_end);

#endif // _MATH__TAXICAB_H_
