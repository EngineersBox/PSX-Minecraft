#pragma once

#ifndef PSX_MINECRAFT_ENGINE_H
#define PSX_MINECRAFT_ENGINE_H

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

#endif // PSX_MINECRAFT_ENGINE_H
