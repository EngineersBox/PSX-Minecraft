#pragma once

#ifndef _PSXMC__RENDER__DEBUG_DEFINES_H_
#define _PSXMC__RENDER__DEBUG_DEFINES_H_

#define isDebugEnabled() defined(PSXMC_DEBUG) && PSXMC_DEBUG == 1

// Render FPS debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_FPS 1
// Render position debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_POS 1
// Render direction debug overlay (1 = enabled, 0 = disabled)
// #define PSXMC_DEBUG_OVERLAY_DIR 0

#endif // _PSXMC__RENDER__DEBUG_DEFINES_H_