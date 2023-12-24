#include <sys/types.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>
#include <smd/smd.h>

#include "display.h"

void initDisplay() {
    ResetGraph(0);
    if (GetVideoMode() == MODE_NTSC) {
        SetDefDispEnv(&db[db_active].display_env, 0, 0, SCREEN_XRES, SCREEN_YRES);
        SetDefDrawEnv(&db[db_active].draw_env, 0, 0, SCREEN_XRES, SCREEN_YRES);
        scSetClipRect(0, 0, SCREEN_XRES, SCREEN_YRES);
        printf("NTSC System.\n");
    } else {
        SetDefDispEnv(&db[db_active].display_env, 0, 0, SCREEN_XRES, 512);
        SetDefDrawEnv(&db[db_active].draw_env, 0, 0, SCREEN_XRES, 512);
        scSetClipRect(0, 0, SCREEN_XRES, 512);
        //disp.screen.y = 20;
        //disp.screen.h = 256;
        printf("PAL System.\n");
    }
    db[db_active].display_env.isinter = 1;
    db[db_active].draw_env.isbg = 1;
    PutDispEnv(&db[db_active].display_env);
    PutDrawEnv(&db[db_active].draw_env);
    ClearOTagR(db[db_active].ordering_table, ORDERING_TABLE_LENGTH);
    ClearOTagR(db[db_active].ordering_table, ORDERING_TABLE_LENGTH);
    db_nextpri = db[0].packet_buffer;
    InitGeom();
    gte_SetGeomScreen(CENTRE_X);
    if (GetVideoMode() == MODE_NTSC) {
        gte_SetGeomOffset(CENTRE_X, CENTRE_Y);
    } else {
        gte_SetGeomOffset(CENTRE_X, CENTRE_Y + 2);
    }
}

void _display() {
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&db[db_active].draw_env);
    DrawOTag(db[db_active].ordering_table + ORDERING_TABLE_LENGTH - 1);
    db_active ^= 1;
    ClearOTagR(db[db_active].ordering_table, ORDERING_TABLE_LENGTH);
    db_nextpri = db[db_active].packet_buffer;
    SetDispMask(1);
}