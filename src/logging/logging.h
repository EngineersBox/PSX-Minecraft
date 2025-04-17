#pragma once

#ifndef PSXMC_LOGGING_H
#define PSXMC_LOGGING_H

#include <stdio.h>

#include "../debug/debug_defines.h"

#if isDebugEnabled()
#define DEBUG_LOG(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEBUG_LOG(x, ...) ({})
#endif

void errorAbort(const char* fmt, ...);

#endif // PSXMC_LOGGING_H
