#pragma once

#ifndef PSXMC_APP_LOGIC_H
#define PSXMC_APP_LOGIC_H

#include <interface99.h>

#include "../util/inttypes.h"
#include "../hardware/counters.h"

typedef struct {
    u32 fps;
    u32 tps;
    Timestamp frame_diff_ms;
} Stats;

#define AppLogic_IFACE \
    vfunc(void, cleanup, VSelf) \
    vfunc(void, init, VSelf, void* ctx) \
    vfunc(void, input, VSelf, const Stats* stats) \
    vfunc(void, update, VSelf, const Stats* stats) \
    vfunc(void, render, VSelf, const Stats* stats)

interface(AppLogic);

#endif // PSXMC_APP_LOGIC_H
