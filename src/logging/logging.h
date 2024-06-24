#pragma once

#ifndef PSX_MINECRAFT_LOGGING_H
#define PSX_MINECRAFT_LOGGING_H

#include <stdio.h>

#include "../render/debug_defines.h"

#if isDebugEnabled()
#define DEBUG_LOG(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEBUG_LOG(x, ...) ({})
#endif

void errorAbort(const char* fmt, ...);

#endif // PSX_MINECRAFT_LOGGING_H