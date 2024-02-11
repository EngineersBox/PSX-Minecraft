#pragma once

#ifndef PSX_MINECRAFT_PRIMITIVE_H
#define PSX_MINECRAFT_PRIMITIVE_H

#include <psxgpu.h>

#include "../render/render_context.h"

typedef struct {
    short v0;
    short v1;
    short v2;
    short v3;
} INDEX;

void lineG2Render(const LINE_G2* line, int ot_entry, RenderContext* ctx);
void polyF4Render(const POLY_F4* pol4, int ot_entry, RenderContext* ctx);

#endif //PSX_MINECRAFT_PRIMITIVE_H
