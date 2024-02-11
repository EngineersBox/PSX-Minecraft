#pragma once

#ifndef PSX_MINECRAFT_DEBUG_H
#define PSX_MINECRAFT_DEBUG_H

#include <psxgte.h>

#include "render_context.h"
#include "../util/circular_buffer.h"

#define SAMPLE_MAX_VALUE 50
#define SAMPLE_WINDOW_SIZE 80
#define SAMPLE_RATE 10

extern CircularBuffer ordering_table_usage;
extern CircularBuffer packet_buffer_usage;

void debugDrawOTUsageGraph(RenderContext* ctx, uint16_t base_screen_x, uint16_t base_screen_y);
void debugDrawPBUsageGraph(RenderContext* ctx, uint16_t base_screen_x, uint16_t base_screen_y);

#endif // PSX_MINECRAFT_DEBUG_H
