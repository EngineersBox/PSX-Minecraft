#pragma once

#ifndef PSXMC_LOADING_BAR_H
#define PSXMC_LOADING_BAR_H

#include <psxgte.h>

#include "../render/render_context.h"

typedef struct {
    struct {
        int32_t x;
        int32_t y;
    } position;
    struct {
        int32_t width;
        int32_t height;
    } dimensions;
    uint32_t value;
    uint32_t maximum;
} ProgressBar;

void progressBarRender(const ProgressBar* bar, int ot_entry, RenderContext* ctx);

#endif // PSXMC_LOADING_BAR_H
