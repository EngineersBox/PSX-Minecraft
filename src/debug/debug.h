#pragma once

#ifndef PSX_MINECRAFT_DEBUG_H
#define PSX_MINECRAFT_DEBUG_H

#include <stdio.h>

#if defined(PSXMC_DEBUG) && PSXMC_DEBUG == 1
#define DEBUG_LOG(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEBUG_LOG(x, ...) ({})
#endif

#endif // PSX_MINECRAFT_DEBUG_H
