#pragma once

#ifndef _PSXMC__GAME_WORLD_GENERATION__NOISE_H_
#define _PSXMC__GAME_WORLD_GENERATION__NOISE_H_

#include "../../../util/inttypes.h"

#define lerp(t, a, b) ({\
    __typeof__(a) _a = (a);\
    _a + ((t) * ((b) - _a) >> 12);\
})

int grad(int hash, int x, int y, int z);
int grad2d(int hash, int x, int y);
int fade(int t);

int noise3d(int x, int y, int z);
int noise2d(int x, int y);

#endif // _PSXMC__GAME_WORLD_GENERATION__NOISE_H_
