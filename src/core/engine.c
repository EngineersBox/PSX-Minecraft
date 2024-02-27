#include "engine.h"

#include "../hardware/counters.h"

void engineInit(Engine* engine, void* ctx) {
    VCALL(*engine->app_logic, init, ctx);
    engine->running = false;
    const HW_CPU_CounterMode mode = (HW_CPU_CounterMode) {
        .fields = {
            .sync = COUNTER_SYNC_FREE_RUN,
            .syncMode = COUNTER_2_SYNCMODE_FREE_RUN,
            .source = COUNTER_2_SOURCE_SYSTEM_CLOCK_DIV_8,
            .interruptRequest = 1
        }
    };
    setCounterMode(COUNTER_2_ID, mode);
}

void engineRun(Engine* engine) {
    uint16_t counter = readCounterValue(COUNTER_2_ID);
    VCALL(*engine->app_logic, cleanup);
}

void engineStart(Engine* engine) {
    engine->running = true;
    engineRun(engine);
}

void engineStop(Engine* engine) {
    engine->running = false;
}