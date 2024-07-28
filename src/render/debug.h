#pragma once

#ifndef PSXMC_DEBUG_H
#define PSXMC_DEBUG_H

#include "render_context.h"
#include "../structure/circular_buffer.h"
#include "../core/app_logic.h"
#include "../core/camera.h"
#include "debug_defines.h"
#include "../util/inttypes.h"

#define SAMPLE_MAX_VALUE 50
#define SAMPLE_WINDOW_SIZE 80
#define SAMPLE_RATE 10

extern CircularBuffer ordering_table_usage;
extern CircularBuffer packet_buffer_usage;

void debugDrawPBUsageGraph(RenderContext* ctx, u16 base_screen_x, u16 base_screen_y);
void drawDebugText(const Stats* stats, const Camera* camera);

#endif // PSXMC_DEBUG_H
