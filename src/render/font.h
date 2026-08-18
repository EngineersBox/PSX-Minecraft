#pragma once

#include "psxgte.h"
#ifndef PSXMC_FONT_H
#define PSXMC_FONT_H

#include <stdbool.h>

#include "../util/inttypes.h"
#include "render_context.h"

#define FONT_CHARACTER_SPRITE_WIDTH 8
#define FONT_CHARACTER_SPRITE_HEIGHT 8
#define FONT_SPRITE_WIDTH 16
#define FONT_SPRITE_HEIGHT 16
#define FONT_SPRITE_V_OFFSET 32

typedef int FontID;

u32 fontStringWidth(const char* string);

void fontPrintCentreOffset(RenderContext* ctx,
                           i16 x_offset,
                           u16 y,
                           u32 fmt_add_bytes,
                           const size_t ot_entry_index,
                           const char* fmt, ...);

void fontLoad();
FontID fontOpen(int x, int y, int w, int h, bool isbg, bool shadow, int n);
void* fontSort(u32* ordering_table,
               void* primitive,
               const u16 x,
               const u16 y,
               const bool shadow,
               const CVECTOR* bg,
               const CVECTOR* border,
               const u8 border_size,
               const char* text);
int fontPrint(FontID id, const char* fmt, ...);
void* fontFlush(FontID id);

#endif // PSXMC_FONT_H
