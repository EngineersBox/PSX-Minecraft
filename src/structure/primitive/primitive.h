#pragma once

#ifndef PSXMC_PRIMITIVE_H
#define PSXMC_PRIMITIVE_H

#include <psxgpu.h>
#include <stdbool.h>

#include "../../render/render_context.h"
#include "../../util/inttypes.h"

typedef struct {
    short v0;
    short v1;
    short v2;
    short v3;
} INDEX;

// Direction: true (1) = right, false (0) = left
INDEX indexRotate90(const INDEX* index, const bool direction);

void lineG2Render(const LINE_G2* line, int ot_entry, RenderContext* ctx);
void lineF2Render(const LINE_F2* line, int ot_entry, RenderContext* ctx);
void polyF4Render(const POLY_F4* pol4, int ot_entry, RenderContext* ctx);
void polyFT4Render(const POLY_FT4* pol4, int ot_entry, RenderContext* ctx);
void tileRender(const TILE* tile, int ot_entry, RenderContext* ctx);
void fillRender(const FILL* fill, int ot_entry, RenderContext* ctx);

#endif //PSXMC_PRIMITIVE_H
