#pragma once

#ifndef PSXMC_FONT_H
#define PSXMC_FONT_H

#include <stdbool.h>

#include "../util/inttypes.h"
#include "render_context.h"

#define FONT_CHARACTER_SPRITE_WIDTH 8
#define FONT_CHARACTER_SPRITE_HEIGHT 8
#define FONT_SPRITE_WIDTH 16
#define FONT_SPRITE_HEIGHT 16

typedef int FontID;

u32 fontStringWidth(const char* string);

void fontPrintCentreOffset(RenderContext* ctx, i32 x_offset, i32 y, u32 fmt_add_bytes, const char* fmt, ...);

void fontLoad();
FontID fontOpen(int x, int y, int w, int h, bool isbg, bool shadow, int n);
void* fontSort(u32* ordering_table, void* primitive, int x, int y, bool shadow, const char* text);
int fontPrint(FontID id, const char* fmt, ...);
void* fontFlush(FontID id);

#endif // PSXMC_FONT_H
