//#pragma once
//
//#ifndef PSX_MINECRAFT_CUBE_H
//#define PSX_MINECRAFT_CUBE_H
//
//#include <psxgte.h>
//
//#include "../primitive/primitive.h"
//
//typedef struct Cube {
//    SVECTOR vertices[8];
//    uint16_t texture_tpage;
//    uint16_t texture_clut;
//} Cube;
//
//const SVECTOR CUBE_NORMS[6] = {
//    { 0, 0, -ONE, 0 },
//    { 0, 0, ONE, 0 },
//    { 0, -ONE, 0, 0 },
//    { 0, ONE, 0, 0 },
//    { -ONE, 0, 0, 0 },
//    { ONE, 0, 0, 0 }
//};
//
//const INDEX CUBE_INDICES[6] = {
//    { 0, 1, 2, 3 },
//    { 4, 5, 6, 7 },
//    { 5, 4, 0, 1 },
//    { 6, 7, 3, 2 },
//    { 0, 2, 5, 7 },
//    { 3, 1, 6, 4 }
//};
//#define CUBE_FACES 6
//
//void cubeRender(Cube* cube);
//
//#endif //PSX_MINECRAFT_CUBE_H
