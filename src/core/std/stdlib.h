#pragma once

#ifndef _CORE_STD__MEMORY_H_
#define _CORE_STD__MEMORY_H_

#include "../../debug/debug_defines.h"
#include <stdlib.h>

#if isDebugTagEnabled(PCSX_ASAN)
#include "../../debug/pcsx.h"
#define malloc pcsx_msanAlloc
#define calloc pcsx_msanCalloc
#define realloc pcsx_msanRealloc
#define free pcsx_msanFree
#endif

#endif // _CORE_STD__MEMORY_H_
