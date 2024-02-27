#include "engine.h"

#include "../hardware/counters.h"

void engineInit(Engine* engine, void* ctx) {
    VCALL(*engine->app_logic, init, ctx);
    engine->running = false;
    const HW_CPU_CounterMode mode = (HW_CPU_CounterMode) {
        .fields = {
            .syncEnable = 1,
            .interruptRequest = 1
        }
    };
    setCounterMode(0, mode);
}

void engineRun(Engine* engine) {
    uint16_t counter = readCounterValue(0);
    VCALL(*engine->app_logic, cleanup);
}

void engineStart(Engine* engine) {
    engine->running = true;
    engineRun(engine);
}

void engineStop(Engine* engine) {
    engine->running = false;
}