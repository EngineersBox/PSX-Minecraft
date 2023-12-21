//#include <sys/types.h>
//#include <stdio.h>
//#include <psxgpu.h>
//#include <psxgte.h>
//#include <inline_c.h>
//#include <smd/smd.h>
//
//#include "display.h"
//
//DISPENV disp;
//DRAWENV draw;
//
//char pribuff[2][131072];
//u_long ordering_table[2][ORDERING_TABLE_LENGTH];
//char* nextpri;
//int db = 0;
//
//MATRIX mtx;
//
//void initDisplay() {
//    ResetGraph(0);
//    if (GetVideoMode() == MODE_NTSC) {
//        SetDefDispEnv(&disp, 0, 0, 640, 480);
//        SetDefDrawEnv(&draw, 0, 0, 640, 480);
//        scSetClipRect(0, 0, 640, 480);
//        printf("NTSC System.\n");
//    } else {
//        SetDefDispEnv(&disp, 0, 0, 640, 512);
//        SetDefDrawEnv(&draw, 0, 0, 640, 512);
//        scSetClipRect(0, 0, 640, 512);
//        //disp.screen.y = 20;
//        //disp.screen.h = 256;
//        printf("PAL System.\n");
//    }
//
//    disp.isinter = 1;
//    draw.isbg = 1;
//
//    PutDispEnv(&disp);
//    PutDrawEnv(&draw);
//
//    ClearOTagR(ordering_table[0], ORDERING_TABLE_LENGTH);
//    ClearOTagR(ordering_table[1], ORDERING_TABLE_LENGTH);
//    nextpri = pribuff[0];
//
//    InitGeom();
//    gte_SetGeomScreen(320);
//
//    if (GetVideoMode() == MODE_NTSC) {
//        gte_SetGeomOffset(320, 240);
//    } else {
//        gte_SetGeomOffset(320, 256);
//    }
//}
//
//void _display() {
//    DrawSync(0);
//    VSync(0);
//
//    PutDrawEnv(&draw);
//    DrawOTag(ordering_table[db] + ORDERING_TABLE_LENGTH - 1);
//
//    db ^= 1;
//    ClearOTagR(ordering_table[db], ORDERING_TABLE_LENGTH);
//    nextpri = pribuff[db];
//
//    SetDispMask(1);
//}