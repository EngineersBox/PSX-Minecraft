#include "render_context.h"

#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>
#include <smd/smd.h>
#include <stdlib.h>

#include "../logging/logging.h"
#include "../math/math_utils.h"

CVECTOR clear_colour = {
    .r = 63,
    .g = 63,//0,
    .b = 63,//127,
    .cd = 0
};
CVECTOR back_colour = {
    .r = 63,
    .g = 63,
    .b = 63,
    .cd = 0
};
CVECTOR far_colour = {
    .r = 63,
    .g = 63,
    .b = 63,
    .cd = 0
};
// Light color matrix
// Each column represents the color matrix of each light source and is
// used as material color when using gte_ncs() or multiplied by a
// source color when using gte_nccs(). 4096 is 1.0 in this matrix
// A column of zeroes effectively disables the light source.
MATRIX lighting_colour = {
    .m = {
        {FIXED_1_4 * 3, 0, 0}, /* Red */
        {FIXED_1_4 * 3, 0, 0}, /* Green */
        {FIXED_1_4 * 3, 0, 0}  /* Blue */
    }
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX lighting_direction = {
    /* X,  Y,  Z */
    .m = {
        {-FIXED_1_2, -FIXED_1_2, FIXED_1_2},
        {0, 0, 0},
        {0, 0, 0}
    }
};

void initRenderContext(RenderContext* ctx) {
    // Reset GPU and install VSync event handler
    ResetGraph(0);
    // Set display and draw environment areas
    // NOTE: display and draw areas must be separate otherwise flickering occurs
    int w;
    int h;
    GPU_VideoMode video_mode = GetVideoMode();
    if (video_mode == MODE_NTSC) {
        w = SCREEN_XRES;
        h = SCREEN_YRES;
        printf("NTSC System.\n");
    } else {
        w = SCREEN_XRES;
        h = 512;
        printf("PAL System.\n");
    }
    // First set of display/draw environments
    SetDefDispEnv(&ctx->db[0].display_env, 0, 0, w, h);
    SetDefDrawEnv(&ctx->db[0].draw_env, 0, h, w, h);
    scSetClipRect(0, 0, w, h);
    // Enable draw areas clear and dither processing
    setRGB0(
        &ctx->db[0].draw_env,
        clear_colour.r,
        clear_colour.g,
        clear_colour.b
    );
    // ctx->db[0].display_env.isinter = 1; // Interlace
    ctx->db[0].draw_env.isbg = 1; // Background
    ctx->db[0].draw_env.dtd = 1; // Dithered
    // Second set of diplay/draw envirnoments
    SetDefDispEnv(&ctx->db[1].display_env, 0, h, w, h);
    SetDefDrawEnv(&ctx->db[1].draw_env, 0, 0, w, h);
    // Enable draw areas clear and dither processing
    setRGB0(
        &ctx->db[1].draw_env,
        clear_colour.r,
        clear_colour.g,
        clear_colour.b
    );
    // ctx->db[1].display_env.isinter = 1; // Interlace
    ctx->db[1].draw_env.isbg = 1; // Background
    ctx->db[1].draw_env.dtd = 1; // Dithered
    // Apply drawing environment to first double buffer
    // PutDispEnv(&ctx->db[ctx->active].display_env);
    PutDrawEnv(&ctx->db[ctx->active].draw_env);
    // Clear both ordering tables to make sure they are clean at start
    ClearOTagR(ctx->db[0].ordering_table, ORDERING_TABLE_LENGTH);
    ClearOTagR(ctx->db[1].ordering_table, ORDERING_TABLE_LENGTH);
    // Set primitive pointer address
    ctx->primitive = ctx->db[0].packet_buffer;
    // Set clip region
    setRECT(&ctx->screen_clip, 0, 0, SCREEN_XRES, SCREEN_YRES);
    // Initialise the GTE
    InitGeom();
    // Set screen depth (basically FOV control, W/2 works best)
    gte_SetGeomScreen(FOV);
    // Set GTE offset (reccomended method of centering)
    if (video_mode == MODE_NTSC) {
        gte_SetGeomOffset(CENTRE_X, CENTRE_Y);
    } else {
        gte_SetGeomOffset(CENTRE_X, CENTRE_Y + 2);
    }
    // Colour used for faces with normal away from
    // light source
    gte_SetBackColor(
        back_colour.r,
        back_colour.g,
        back_colour.b
    );
    // Tint to fog colour
    gte_SetFarColor(
        far_colour.r,
        far_colour.g,
        far_colour.b
    );
    // Set light ambient color and light colorma trix
    gte_SetColorMatrix(&lighting_colour);
}

void swapBuffers(RenderContext* ctx) {
    // Wait for GPU to finish drawing and vertical retrace
    DrawSync(0);
    VSync(0);
    // Swap buffers
    ctx->active ^= 1;
    ctx->primitive = ctx->db[ctx->active].packet_buffer;
    // Clear the OT of the next frame
    ClearOTagR(ctx->db[ctx->active].ordering_table, ORDERING_TABLE_LENGTH);
    // Apply display/drawing environments
    PutDrawEnv(&ctx->db[ctx->active].draw_env);
    PutDispEnv(&ctx->db[ctx->active].display_env);
    // Enable display
    SetDispMask(1);
    // Start drawing the OT of the last buffer
    DrawOTag(ctx->db[1 - ctx->active].ordering_table + (ORDERING_TABLE_LENGTH - 1));
}

void renderClearConstraintsIndex(RenderContext* ctx, uint32_t index) {
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    // Zeroed fields indicates clearing/reset any applied texture windows
    const RECT tex_window = {
        .x = 0,
        .y = 0,
        .w = 0,
        .h = 0
    };
    setTexWindow(ptwin, &tex_window);
    uint32_t* ot_object = allocateOrderingTable(ctx, index);
    addPrim(ot_object, ptwin);
}

void renderClearConstraints(RenderContext* ctx) {
    renderClearConstraintsIndex(ctx, 0);
}

char* allocatePrimitive(RenderContext* ctx, const size_t size) {
    size_t free_space = PACKET_BUFFER_LENGTH;
    free_space -= (uintptr_t) ctx->primitive - (uintptr_t) ctx->db[ctx->active].packet_buffer;
    if (free_space < size) {
        errorAbort("[ERROR] Not enough space in packet buffer: 0x%x < 0x%x\n", free_space, size);
    }
    void* prev = ctx->primitive;
    ctx->primitive += size;
    return prev;
}

void freePrimitive(RenderContext* ctx, const size_t size) {
    const size_t allocated_space = (uintptr_t) ctx->primitive
        - (uintptr_t) ctx->db[ctx->active].packet_buffer;
    if (allocated_space < size) {
        errorAbort(
            "[ERROR] Free size is larger than allocated space in packet buffer: 0x%x < 0x%x\n",
            allocated_space,
            size
        );
    }
    ctx->primitive -= size;
}

uint32_t* allocateOrderingTable(RenderContext* ctx, const size_t size) {
    if (size >= ORDERING_TABLE_LENGTH) {
        errorAbort("[ERROR] Not enough space in ordering table: 0x%x >= 0x%x\n", size, ORDERING_TABLE_LENGTH);
    }
    return ctx->db[ctx->active].ordering_table + size;
}
