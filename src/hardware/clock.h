#pragma once

#ifndef PSX_MINECRAFT_CLOCK_H
#define PSX_MINECRAFT_CLOCK_H

#define OSCILLATOR_HZ 44100
// As per:
// - https://github.com/grumpycoders/pcsx-redux/blob/7bb155647ae3a430554305173aae6cb5dd48cf65/src/core/psxemulator.h#L226
// - https://psx-spx.consoledev.net/pinouts/#pinouts-clk-pinouts
#define CPU_CLOCK_HZ (OSCILLATOR_HZ * 0x300)
#define VIDEO_CLOCK_HZ (OSCILLATOR_HZ * 0x300 * (11 / 7))

// Roughly 0x844d, which is smaller than 0xffff
#define CPU_CYCLES_PER_MS (CPU_CLOCK_HZ / 1000)

#endif // PSX_MINECRAFT_CLOCK_H