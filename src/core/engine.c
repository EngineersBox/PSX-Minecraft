#include "engine.h"

#include <psxetc.h>

#include "../util/math_utils.h"
#include "../hardware/counters.h"

volatile uint32_t time_ms = 0;

void irqCallbackTimeMsIncrement() {
    time_ms++;
}

void engineInit(Engine* engine, void* ctx) {
    VCALL(*engine->app_logic, init, ctx);
    engine->running = false;
    const HW_CPU_CounterMode mode = (HW_CPU_CounterMode) {
        .fields = {
            .sync = COUNTER_SYNC_FREE_RUN,
            .source = COUNTER_0_SOURCE_SYSTEM_CLOCK,
            .reset = COUNTER_RESET_AT_TARGET,
            .irqAtTarget = true,
            .irqInvocationType = COUNTER_IRQ_INVOCATION_REPEATED
        }
    };
    setCounterMode(COUNTER_0_ID, mode);
    setCounterTarget(COUNTER_0_ID, CPU_CYCLES_PER_MS);
    InterruptCallback(IRQ_TIMER0, &irqCallbackTimeMsIncrement);
}

void engineRun(Engine* engine) {
    engine->running = true;
    uint32_t seconds = 0;
    uint16_t initial_time = time_ms;
    const uint32_t time_t = 1000 / engine->target_tps;
    const uint32_t time_f = engine->target_fps > 0 ? 1000 / engine->target_fps : 0;
    uint32_t delta_update = 0;
    uint32_t delta_fps = 0;
    Stats stats = {
        .fps = 0,
        .tps = 0,
        .diff_ms = 0
    };
    uint32_t frames = 0;
    uint32_t ticks = 0;
    while (engine->running) {
        const uint32_t now  = time_ms;
        const uint32_t diff = positiveModulo(now - initial_time, FIXED_POINT_MAX) << FIXED_POINT_SHIFT;
        delta_update += diff / time_t;
        delta_fps += diff / time_f;
        if (engine->target_fps <= 0 || delta_fps >= ONE) {
            VCALL(*engine->app_logic, input, &stats);
        }
        if (delta_update >= ONE) {
            VCALL(*engine->app_logic, update, &stats);
            delta_update -= ONE;
            ticks++;
        }
        if (engine->target_fps <= 0 || delta_fps >= ONE) {
            VCALL(*engine->app_logic, render, &stats);
            delta_fps -= ONE;
            frames++;
        }
        initial_time = now;
        if (seconds + 1 == now / 1000) {
            stats.fps = frames;
            stats.tps = ticks;
            stats.diff_ms = diff;
            frames = 0;
            ticks = 0;
            seconds++;
        }
    }
    VCALL(*engine->app_logic, cleanup);
}

void engineStop(Engine* engine) {
    engine->running = false;
}