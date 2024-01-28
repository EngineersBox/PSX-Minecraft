#pragma once

#ifndef PSX_MINECRAFT_RENDER_CONTEXT_H
#define PSX_MINECRAFT_RENDER_CONTEXT_H

#include <psxgpu.h>
#include <psxgte.h>

// DB struct array sizes
#define ORDERING_TABLE_LENGTH (1 << 12)
#define PACKET_BUFFER_LENGTH (1 << 16)

// Screen resolution
#define SCREEN_XRES	320
#define SCREEN_YRES	240

// Screen center position
#define CENTRE_X (SCREEN_XRES >> 1)
#define CENTRE_Y (SCREEN_YRES >> 1)

extern CVECTOR clear_colour;

// Double buffer structure
typedef struct {
    DISPENV display_env;
    DRAWENV draw_env;
    uint32_t ordering_table[ORDERING_TABLE_LENGTH];
    char packet_buffer[PACKET_BUFFER_LENGTH];
} DB;

typedef struct {
    char* primitive;
    RECT screen_clip;
    DB db[2];
    uint8_t active;
} RenderContext;

void initRenderContext(RenderContext* ctx);

void swapBuffers(RenderContext* ctx);

void renderClearConstraints(RenderContext* ctx);

char* allocatePrimitive(RenderContext* ctx, size_t size);
void freePrimitive(RenderContext* ctx, size_t size);
uint32_t* allocateOrderingTable(RenderContext* ctx, size_t size);

#endif // PSX_MINECRAFT_RENDER_CONTEXT_H
