#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>
#include <smd/smd.h>

#include "display.h"

void initDisplay(DisplayContext* ctx) {
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
    SetDefDrawEnv(&ctx->db[0].draw_env, w, 0, w, h);
    scSetClipRect(0, 0, w, h);
    // Enable draw areas clear and dither processing
    setRGB0(&ctx->db[0].draw_env, 63, 0, 127);
    // ctx->db[0].display_env.isinter = 1; // Interlace
    ctx->db[0].draw_env.isbg = 1; // Background
    ctx->db[0].draw_env.dtd = 1; // Dithered
    // Second set of diplay/draw envirnoments
    SetDefDispEnv(&ctx->db[1].display_env, w, 0, w, h);
    SetDefDrawEnv(&ctx->db[1].draw_env, 0, 0, w, h);
    // Enable draw areas clear and dither processing
    setRGB0(&ctx->db[1].draw_env, 63, 0, 127);
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
    // Set screen depth (basically FOV control, W/2 works best
    gte_SetGeomScreen(CENTRE_X);
    // Set GTE offset (reccomended method of centering)
    if (video_mode == MODE_NTSC) {
        gte_SetGeomOffset(CENTRE_X, CENTRE_Y);
    } else {
        gte_SetGeomOffset(CENTRE_X, CENTRE_Y + 2);
    }
}

void display(DisplayContext* ctx) {
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