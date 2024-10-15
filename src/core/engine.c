#include "engine.h"

#include <psxetc.h>
#include <psxapi.h>
#include <stdio.h>

#include "../math/math_utils.h"
#include "../hardware/counters.h"

volatile u32 time_ms = 0;

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
    const Timestamp ms_per_tick = 1000 / engine->target_tps;
    engine->running = true;
    Timestamp previous = time_ms;
    u32 delta = 0;
    Stats stats = {
        .fps = 0,
        .tps = 0,
        .frame_diff_ms = 0
    };
    u32 frames = 0;
    u32 ticks = 0;
    Timestamp total_elapsed = 0;
    // TODO: Figure out why this is needed when the
    //       input call is moved within the for loop
    //       inside this engine loop.
    VCALL(*engine->app_logic, input, &stats);
    while (engine->running) {
        const Timestamp current = time_ms;
        const Timestamp elapsed = positiveModulo(current - previous, FIXED_POINT_MAX);
        previous = current;
        delta += elapsed;
        total_elapsed += elapsed;
        for (; delta >= ms_per_tick; delta -= ms_per_tick) {
            VCALL(*engine->app_logic, input, &stats);
            VCALL(*engine->app_logic, update, &stats);
            ticks++;
        }
        VCALL(*engine->app_logic, render, &stats);
        frames++;
        if (total_elapsed >= 1000) {
            stats.fps = frames;
            stats.tps = ticks;
            stats.frame_diff_ms = elapsed;
            frames = 0;
            ticks = 0;
            total_elapsed = 0;
        }
    }
    VCALL(*engine->app_logic, cleanup);
}

void engineRunOld(Engine* engine) {
    engine->running = true;
    u32 seconds = 0;
    Timestamp initial_time = time_ms;
    const u32 time_t = 1000 / engine->target_tps;
    const u32 time_f = engine->target_fps > 0 ? 1000 / engine->target_fps : 0;
    u32 delta_update = 0;
    u32 delta_fps = 0;
    Stats stats = {
        .fps = 0,
        .tps = 0,
        .frame_diff_ms = 0
    };
    u32 frames = 0;
    u32 ticks = 0;
    while (engine->running) {
        const Timestamp now  = time_ms;
        const Timestamp diff = positiveModulo(now - initial_time, FIXED_POINT_MAX) << FIXED_POINT_SHIFT;
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
            stats.frame_diff_ms = diff >> FIXED_POINT_SHIFT;
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
