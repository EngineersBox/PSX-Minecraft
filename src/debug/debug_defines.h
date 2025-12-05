#pragma once

#ifndef _PSXMC__RENDER__DEBUG_DEFINES_H_
#define _PSXMC__RENDER__DEBUG_DEFINES_H_

#define isDebugEnabled() defined(PSXMC_DEBUG) && PSXMC_DEBUG == 1
#define isDebugFlagEnabled(suffix) (isDebugEnabled() && defined(PSXMC_DEBUG_##suffix) && PSXMC_DEBUG_##suffix)

// Render FPS debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_FPS 1
// Render position debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_POS 1
// Render direction debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_DIR 0
// Render facing direction (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_FACING 0
// Render memory usage debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_MEM 1
// Render world data like day count, time of day and weather (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_WORLD 1
// Render critical path duration tree
// #define PSXMC_DEBUG_OVERLAY_DURATION_TREE 1

// Player no-clip
// #define PSXMC_DEBUG_PLAYER_NOCLIP 0

// PCSX-Redux MIPS API
// Address sanitiser
// #define PSXMC_DEBUG_PCSX_ASAN 0
// Kernel checker
// #define PSXMC_DEBUG_PCSX_KERNEL_CHECKER 0
// Debugging utils
// #define PSXMC_DEBUG_PCSX_UTILS 0

#endif // _PSXMC__RENDER__DEBUG_DEFINES_H_
