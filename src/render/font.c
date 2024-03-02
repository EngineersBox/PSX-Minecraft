#include "font.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

uint32_t fontStringWidth(const char* string) {
    return FONT_SPRITE_WIDTH * strlen(string);
}

void fontPrintCentreOffset(RenderContext* ctx,
                           const int32_t x_offset,
                           const int32_t y,
                           const uint32_t fmt_add_bytes,
                           const char* fmt, ...) {
    const uint32_t raw_length = strlen(fmt) + 1;
    char* buf = (char*) calloc(raw_length + fmt_add_bytes, sizeof(*buf));
    memset(buf, 0, raw_length + fmt_add_bytes);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(
        buf,
        raw_length + fmt_add_bytes,
        fmt,
        ap
    );
    va_end(ap);
    ctx->primitive = FntSort(
        allocateOrderingTable(ctx, 0),
        ctx->primitive,
        x_offset - (fontStringWidth(buf) / 2),
        y,
        buf
    );
}