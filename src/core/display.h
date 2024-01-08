#pragma once

#ifndef PSX_MINECRAFT_DISPLAY_H
#define PSX_MINECRAFT_DISPLAY_H

#include <psxgpu.h>

// DB struct array sizes
#define ORDERING_TABLE_LENGTH 1024
#define PACKET_BUFFER_LENGTH 8096

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
    int active;
    char* primitive;
    RECT screen_clip;
    DB db[2];
} DisplayContext;

void initDisplay(DisplayContext* ctx);
void display(DisplayContext* ctx);

#endif //PSX_MINECRAFT_DISPLAY_H
