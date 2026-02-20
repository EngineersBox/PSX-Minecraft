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

// (2-(2/(1+tan(45deg)))) * 4096 = 4096
#define TRAD_45_DEG 4096
// (2-(2/(1+tan(50deg)))) * 4096 = 4454.35356...
#define TRAD_50_DEG 4454
// (2-(2/(1+tan(60deg)))) * 4096 = 5193.51989...
#define TRAD_60_DEG 5194
// (2-(2/(1+tan(70deg)))) * 4096 = 6005.99616...
#define TRAD_70_DEG 6006
// (2-(2/(1+tan(80deg)))) * 4096 = 6964.05007
#define TRAD_80_DEG 6964
// Note: tan(90) is undefined, so we take the limit
// lim x->90 (2-(2/(1+tan(x deg)))) * 4096 = 8192
#define TRAD_90_DEG 8192

TRad tcabAngle(const fixedi32 x, const fixedi32 y);

bool tcabAngleInRange(const TRad ref,
                      const TRad angle,
                      const TRad query);

#endif // _MATH__TAXICAB_H_
