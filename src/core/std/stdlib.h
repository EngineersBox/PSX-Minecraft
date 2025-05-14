#pragma once

#ifndef _CORE_STD__MEMORY_H_
#define _CORE_STD__MEMORY_H_

#include "../debug/debug_defines.h"

#if isDebugTagEnabled(PCSX_ASAN)
#include "../debug/pcsx.h"
#define malloc pcsx_msanAlloc
#define calloc pcsx_msanAlloc
#define realloc pcsx_msanRealloc
#define free pcsx_msanFree
__attribute__((constructor)) void __msan_init() {
    pcsx_initMsan();
}
#endif

#include <stdlib.h>

#endif // _CORE_STD__MEMORY_H_
