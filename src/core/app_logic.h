#pragma once

#ifndef PSXMC_APP_LOGIC_H
#define PSXMC_APP_LOGIC_H

#include <interface99.h>
#include <stdint.h>

typedef struct {
    uint32_t fps;
    uint32_t tps;
    uint32_t diff_ms;
} Stats;

#define AppLogic_IFACE \
    vfunc(void, cleanup, VSelf) \
    vfunc(void, init, VSelf, void* ctx) \
    vfunc(void, input, VSelf, const Stats* stats) \
    vfunc(void, update, VSelf, const Stats* stats) \
    vfunc(void, render, VSelf, const Stats* stats)

interface(AppLogic);

#endif // PSXMC_APP_LOGIC_H
