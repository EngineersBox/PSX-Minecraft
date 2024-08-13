#include "engine.h"

#include <psxetc.h>
#include <psxapi.h>

#include "../math/math_utils.h"
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
            .source = COUNTER_2_SOURCE_SYSTEM_CLOCK,
            .reset = COUNTER_RESET_AT_TARGET,
            .irqAtTarget = true,
            .irqInvocationType = COUNTER_IRQ_INVOCATION_REPEATED
        }
    };
    setCounterMode(COUNTER_2_ID, mode);
    setCounterTarget(COUNTER_2_ID, CPU_CYCLES_PER_MS);
    EnterCriticalSection();
    InterruptCallback(IRQ_TIMER2, &irqCallbackTimeMsIncrement);
    ExitCriticalSection();
}

void engineRun(Engine* engine) {
    const u32 ms_per_update = 1000 / engine->target_tps;
    engine->running = true;
    u32 previous = time_ms;
    u32 lag = 0;
    Stats stats = {
        .fps = 0,
        .tps = 0,
        .diff_ms = 0
    };
    u32 frames = 0;
    u32 ticks = 0;
    u32 total_elapsed = 0;
    while (engine->running) {
        u32 current = time_ms;
        u32 elapsed = positiveModulo(current - previous, FIXED_POINT_MAX);
        previous = current;
        lag += elapsed;
        total_elapsed += elapsed;
        VCALL(*engine->app_logic, input, &stats);
        while (lag >= ms_per_update) {
            VCALL(*engine->app_logic, update, &stats);
            lag -= ms_per_update;
            ticks++;
        }
        VCALL(*engine->app_logic, render, &stats);
        frames++;
        if (total_elapsed >= 1000) {
            stats.fps = frames;
            stats.tps = ticks;
            frames = 0;
            ticks = 0;
            total_elapsed = 0;
        }
    }
    VCALL(*engine->app_logic, cleanup);
}

void engineRunOld(Engine* engine) {
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
