#pragma once

#ifndef PSXMC_RENDER_CONTEXT_H
#define PSXMC_RENDER_CONTEXT_H

#include <psxgpu.h>
#include <psxgte.h>

#include "../core/camera.h"
#include "transforms.h"

// Framebuffer struct array sizes
#define ORDERING_TABLE_LENGTH (1 << 13)
#define PACKET_BUFFER_LENGTH (1 << 16)

// Screen resolution
#define SCREEN_XRES	320
#define SCREEN_YRES	240
// TODO: Fix support for other resolutions
// #define SCREEN_XRES 640
// #define SCREEN_YRES 480

// Screen center position
#define CENTRE_X (SCREEN_XRES >> 1)
#define CENTRE_Y (SCREEN_YRES >> 1)

#define FOV (SCREEN_XRES >> 1)

extern CVECTOR clear_colour;
extern CVECTOR back_colour;
extern CVECTOR far_colour;
extern MATRIX lighting_colour;
extern MATRIX lighting_direction;

// Double buffer structure
typedef struct {
    DISPENV display_env;
    DRAWENV draw_env;
    uint32_t ordering_table[ORDERING_TABLE_LENGTH];
    char packet_buffer[PACKET_BUFFER_LENGTH];
} Framebuffer;

typedef struct {
    char* primitive;
    RECT screen_clip;
    Framebuffer db[2];
    uint8_t active;
    Camera* camera;
} RenderContext;

void initRenderContext(RenderContext* ctx);

void swapBuffers(RenderContext* ctx);

void renderClearConstraintsIndex(RenderContext* ctx, uint32_t index);
 void renderClearConstraints(RenderContext* ctx);

void renderCtxBindMatrix(RenderContext* ctx,
                         Transforms* transforms,
                         const SVECTOR* rotation,
                         const VECTOR* translation);
INLINE void renderCtxUnbindMatrix() {
    PopMatrix();
}

char* allocatePrimitive(RenderContext* ctx, size_t size);
void freePrimitive(RenderContext* ctx, size_t size);
uint32_t* allocateOrderingTable(RenderContext* ctx, size_t depth);

#endif // PSXMC_RENDER_CONTEXT_H
