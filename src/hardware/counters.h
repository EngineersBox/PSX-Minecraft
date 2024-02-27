#pragma once

#ifndef PSX_MINECRAFT_COUNTERS_H
#define PSX_MINECRAFT_COUNTERS_H

#include <stdint.h>
#include <hwregs_c.h>

typedef union {
    struct {
        // 0 = free run, 1 = sync via bit 1-2
        uint8_t sync: 1;
        // Synchronization Modes for Counter 0:
        //   0 = Pause counter during Hblank(s)
        //   1 = Reset counter to 0000h at Hblank(s)
        //   2 = Reset counter to 0000h at Hblank(s) and pause outside of Hblank
        //   3 = Pause until Hblank occurs once, then switch to Free Run
        // Synchronization Modes for Counter 1:
        //   Same as above, but using Vblank instead of Hblank
        // Synchronization Modes for Counter 2:
        //   0 or 3 = Stop counter at current value (forever, no h/v-blank start)
        //   1 or 2 = Free Run (same as when Synchronization Disabled)
        uint8_t syncMode: 2;
        // Reset counter to 0x0000, 0 = after counter = 0xffff, 1 = after counter = target
        uint8_t reset: 1;
        // IRQ when counter = target, 0 = disable, 1 = enable
        uint8_t irqAtTarget: 1;
        // IRQ when counter = 0xffff, 0 = disable, 1 = enable
        uint8_t irqAtMax: 1;
        // 0 = one shot, 1 = repeatedly
        uint8_t irqOnceRepeat: 1;
        // 0 = short bit10 = 0 pulse, 1 = toggle bit10 on/off
        uint8_t irqMode: 1;
        // Counter 0:  0 or 2 = System Clock,  1 or 3 = Dotclock
        // Counter 1:  0 or 2 = System Clock,  1 or 3 = Hblank
        // Counter 2:  0 or 1 = System Clock,  2 or 3 = System Clock/8
        uint8_t source: 2;
        // 0 = yes, 1 = no
        uint8_t interruptRequest: 1;
        // 0 = yes, 1 = no
        uint8_t reachedTargetValue: 1;
        // 0 = yes, 1 = no
        uint8_t reachedFFFFValue: 1;
        uint8_t __alwaysZero: 2;
    } fields;
    uint16_t bits;
} HW_CPU_CounterMode;

// Counter IDs
#define COUNTER_0_ID 0
#define COUNTER_1_ID 1
#define COUNTER_2_ID 2

// All counter sync state
#define COUNTER_SYNC_FREE_RUN 0
#define COUNTER_SYNC_BIT1_2 1

// Counter 0 sync modes
#define COUNTER_0_SYNCMODE_PAUSE_ON_HBLANK 0
#define COUNTER_0_SYNCMODE_RESET_TO_0000_AT_HBLANK 1
#define COUNTER_0_SYNCMODE_RESET_TO_0000_AT_HBLANK_PAUSE_OUTSIDE 2
#define COUNTER_0_SYNCMODE_PAUSE_UNTIL_HBLANK_ONCE_THEN_FREE_RUN 3

// Counter 1 sync modes
#define COUNTER_1_SYNCMODE_PAUSE_ON_VBLANK 0
#define COUNTER_1_SYNCMODE_RESET_TO_0000_AT_VBLANK 1
#define COUNTER_1_SYNCMODE_RESET_TO_0000_AT_VBLANK_PAUSE_OUTSIDE 2
#define COUNTER_1_SYNCMODE_PAUSE_UNTIL_VBLANK_ONCE_THEN_FREE_RUN 3

// Counter 2 sync modes
#define COUNTER_2_SYNCMODE_STOP_AT_CURRENT_VALUE 0
#define COUNTER_2_SYNCMODE_FREE_RUN 1

// Counter 0 sources
#define COUNTER_0_SOURCE_SYSTEM_CLOCK 0
#define COUNTER_0_SOURCE_DOT_CLOCK 1

// Counter 1 sources
#define COUNTER_1_SOURCE_SYSTEM_CLOCK 0
#define COUNTER_1_SOURCE_HBLANK 1

// Counter 2 sources
#define COUNTER_2_SOURCE_SYSTEM_CLOCK 0
#define COUNTER_2_SOURCE_SYSTEM_CLOCK_DIV_8 2

#define getCounterMode(id) (HW_CPU_CounterMode) { .bits = TIMER_CTRL(id) }
#define setCounterMode(id, mode) TIMER_CTRL(id) = mode.bits;
#define readCounterValue(id) TIMER_VALUE(id)

#endif // PSX_MINECRAFT_COUNTERS_H
