#include "font.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

typedef struct {
	char* txtbuff;
	char* txtnext;
	char* pribuff;
	int16_t	x, y;
	int16_t	w, h;
	int	bg, maxchars;
} FontStream;

static FontStream font_stream[8];
static int font_nstreams = 0;

uint16_t font_current_tpage;
uint16_t font_current_clut;

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

// Referenced implementations are from Lameguy64 and spicyjpeg
// in the Psn00bSDK: <https://github.com/Lameguy64/PSn00bSDK/blob/master/libpsn00b/psxgpu/font.c>
// I've modified it to work with a more general front sprite
// sheet with complete ASCII and extended character + symbol
// sets.

FontID fontOpen(const int x,
			    const int y,
			    const int w,
			    const int h,
			    const int isbg,
			    int n) {
	_sdk_validate_args((w > 0) && (h > 0) && (n > 0), -1);
	// Initialize a text stream
	font_stream[font_nstreams].x = x;
	font_stream[font_nstreams].y = y;
	font_stream[font_nstreams].w = w;
	font_stream[font_nstreams].h = h;
	font_stream[font_nstreams].txtbuff = (char*) malloc(n+ 1 );
	int i = (sizeof(SPRT_8) * n) + sizeof(DR_TPAGE);
	if (isbg) {
		i += sizeof(TILE);
	}
	font_stream[font_nstreams].pribuff = (char*) malloc(i);
	font_stream[font_nstreams].maxchars = n;
	font_stream[font_nstreams].txtbuff[0] = 0x0;
	font_stream[font_nstreams].txtnext = font_stream[font_nstreams].txtbuff;
	font_stream[font_nstreams].bg = isbg;
	n = font_nstreams;
	font_nstreams++;
	return n;

}

int fontPrint(FontID id, const char* fmt, ...) {
	_sdk_validate_args((id < font_nstreams) && fmt, -1);
	va_list ap;
	if (id < 0) {
		id = font_nstreams - 1;
	}
	int n = strlen(font_stream[id].txtbuff);
	if (n >= font_stream[id].maxchars) {
		return n;
	}
	va_start(ap, fmt);
	n = vsnprintf(
		font_stream[id].txtnext,
		font_stream[id].maxchars - n,
		fmt,
		ap
	);
	font_stream[id].txtnext += n;
	va_end(ap);
	return strlen(font_stream[id].txtbuff);

}

void* fontFlush(FontID id) {
	_sdk_validate_args(id < font_nstreams, 0);
	if (id < 0) {
		id = font_nstreams-1;
	}
	int sx = font_stream[id].x;
	int sy = font_stream[id].y;
	const char* text = font_stream[id].txtbuff;
	char* opri = font_stream[id].pribuff;
	// Create TPage primitive
	DR_TPAGE* tpage = (DR_TPAGE*)opri;
	setDrawTPage(tpage, 0, 0, font_current_tpage);
	SPRT_8 *sprt;
	// Create a black rectangle background when enabled
	if (font_stream[id].bg) {
		opri += sizeof(DR_TPAGE);
		TILE* tile = (TILE*)opri;
		setTile(tile);
		if (font_stream[id].bg == 2) {
			setSemiTrans(tile, 1);
		}
		setXY0(tile, font_stream[id].x, font_stream[id].y);
		setWH(tile, font_stream[id].w, font_stream[id].h);
		setRGB0(tile, 0, 0, 0);
		setaddr(tpage, tile);
		opri = (char*)tile;
		sprt = (SPRT_8*)(opri+sizeof(TILE));
	} else {
		sprt = (SPRT_8*)(opri+sizeof(DR_TPAGE));
	}
	// Create the sprite primitives
	while (*text != 0) {
		if (*text == '\n' || sx-font_stream[id].x > font_stream[id].w - 8) {
			sx = font_stream[id].x;
			sy += 8;
			if (*text == '\n') {
				text++;
			}
			continue;
		}
		if (sy-font_stream[id].y > font_stream[id].h - 8) {
			break;
		}
		int i = toupper(*text) - ' ';
		if (i > 0) {
			i--;
			setSprt8(sprt);
			setShadeTex(sprt, 1);
			setSemiTrans(sprt, 1);
			setXY0(sprt, sx, sy);
			setUV0(sprt, (i % 16) * 8, (i / 16) * 8);
			sprt->clut = font_current_clut;
			setaddr(opri, sprt);
			opri = (char*)sprt;
			sprt++;
		}
		sx += 8;
		text++;
	}
	// Set a terminator value to the last primitive
	termPrim(opri);
	// Draw the primitives
	DrawSync(0);
	DrawOTag((uint32_t*)font_stream[id].pribuff);
	DrawSync(0);
	font_stream[id].txtnext = font_stream[id].txtbuff;
	font_stream[id].txtbuff[0] = 0;
	return (void*) font_stream[id].pribuff;
}

void* fontSort(u32* ordering_table,
			   void* primitive,
			   const int x,
			   const int y,
			   const char *text) {
	_sdk_validate_args(ordering_table && primitive, 0);
	SPRT_8 *sprt = (SPRT_8*)primitive;
	int sx = x;
	int sy = y;
	while (*text != 0) {
		if (*text == '\n') {
			sx = x;
			sy += 8;
			text++;
			continue;
		}
		int i = toupper(*text) - ' ';
		if (i > 0) {
			i--;
			setSprt8(sprt);
			setShadeTex(sprt, 1);
			setSemiTrans(sprt, 1);
			setXY0(sprt, sx, sy);
			setUV0(sprt, (i % 16) * 8, (i / 16) * 8);
			sprt->clut = font_current_clut;
			addPrim(ordering_table, sprt);
			sprt++;
		}
		sx += 8;
		text++;
	}
	primitive = (char*)sprt;
	DR_TPAGE* tpage = (DR_TPAGE*)primitive;
	tpage->code[0] = font_current_tpage;
	setlen(tpage, 1);
	setcode(tpage, 0xe1);
	addPrim(ordering_table, primitive);
	primitive += sizeof(DR_TPAGE);
	return (void *) primitive;
}