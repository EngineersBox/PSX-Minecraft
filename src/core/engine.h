#pragma once

#ifndef PSXMC_ENGINE_H
#define PSXMC_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

#include "app_logic.h"

typedef struct {
    AppLogic* app_logic;
    volatile bool running;
    uint8_t target_fps;
    uint8_t target_tps;
} Engine;

void engineInit(Engine* engine, void* ctx);

void engineRun(Engine* engine);
void engineStop(Engine* engine);

#endif // PSXMC_ENGINE_H
