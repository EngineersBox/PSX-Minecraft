#pragma once

#ifndef PSX_MINECRAFT_MEMORY_H
#define PSX_MINECRAFT_MEMORY_H

#include "inttypes.h"

#define memberSize(type, member) sizeof(((type *)0)->member)
#define zeroed(ptr) (*(ptr)) = (__typeof__(*ptr)){0}

void humanSize(u64 bytes, char** suffix, u32* whole, u32 *frac);

#endif // PSX_MINECRAFT_MEMORY_H
