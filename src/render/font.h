#pragma once

#ifndef PSX_MINECRAFT_FONT_H
#define PSX_MINECRAFT_FONT_H

#include <stdint.h>

#include "render_context.h"

#define FONT_SPRITE_WIDTH 8
#define FONT_SPRITE_HEIGHT 8

uint32_t fontStringWidth(const char* string);

void fontPrintCentreOffset(RenderContext* ctx, int32_t x_offset, int32_t y, uint32_t fmt_add_bytes, const char* fmt, ...);

#endif // PSX_MINECRAFT_FONT_H
