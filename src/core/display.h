#pragma once

#ifndef PSX_MINECRAFT_DISPLAY_H
#define PSX_MINECRAFT_DISPLAY_H

#include <sys/types.h>
#include <psxgte.h>
#include <psxgpu.h>

// DB struct array sizes
#define ORDERING_TABLE_LENGTH 256
#define PACKET_BUFFER_LENGTH 1024

// Screen resolution
#define SCREEN_XRES	640
#define SCREEN_YRES	480

// Screen center position
#define CENTRE_X	SCREEN_XRES >> 1
#define CENTRE_Y	SCREEN_YRES >> 1

// Double buffer structure
typedef struct {
    DISPENV display_env;
    DRAWENV draw_env;
    uint32_t ordering_table[ORDERING_TABLE_LENGTH];
    char packet_buffer[PACKET_BUFFER_LENGTH];
} DB;

// Double buffer variables
DB db[2];
int db_active = 0;
char *db_nextpri;

#endif //PSX_MINECRAFT_DISPLAY_H
