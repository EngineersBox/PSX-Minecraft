#include "font.h"

#include <string.h>
#include "../core/std/stdlib.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "../logging/logging.h"
#include "../resources/assets.h"
#include "../resources/asset_indices.h"

typedef struct {
	char* txtbuff;
	char* txtnext;
	char* pribuff;
	i16	x, y;
	i16	w, h;
	u8 bg: 1;
	u8 shadow: 1;
	u8 _pad: 6;
	u32 maxchars;
} FontStream;

static FontStream font_stream[8];
static int font_nstreams = 0;

Texture* font_current;

u32 fontStringWidth(const char* string) {
    return FONT_CHARACTER_SPRITE_WIDTH * strlen(string);
}

void fontPrintCentreOffset(RenderContext* ctx,
                           const i32 x_offset,
                           const i32 y,
                           const u32 fmt_add_bytes,
                           const char* fmt, ...) {
    const u32 raw_length = strlen(fmt) + 1;
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
    ctx->primitive = fontSort(
        allocateOrderingTable(ctx, 0),
        ctx->primitive,
        x_offset - (fontStringWidth(buf) / 2),
        y,
        true,
        buf
    );
}

// Referenced implementations are from Lameguy64 and spicyjpeg
// in the Psn00bSDK: <https://github.com/Lameguy64/PSn00bSDK/blob/master/libpsn00b/psxgpu/font.c>
// I've modified it to work with a more general font sprite
// sheet with complete ASCII and extended character + symbol
// sets.

void fontLoad() {
	if (!assets_loaded) {
		errorAbort("[ERROR] Cannot load font, assets have not been loaded\n");
	}
	font_current = &textures[ASSET_TEXTURE__STATIC__FONT];
	DrawSync(0);
	// Clear previously opened text streams
	if (font_nstreams) {
		for(int i = 0; i < font_nstreams; i++) {
			free(font_stream[i].txtbuff);
			free(font_stream[i].pribuff);
		}
		font_nstreams = 0;
	}
}

FontID fontOpen(const int x,
			    const int y,
			    const int w,
			    const int h,
			    const bool isbg,
			    const bool shadow,
			    int n) {
	// TODO: Restrict font texture to 128x128 since we only support that size
	_sdk_validate_args((w > 0) && (h > 0) && (n > 0), -1);
	// Initialize a text stream
	font_stream[font_nstreams].x = x;
	font_stream[font_nstreams].y = y;
	font_stream[font_nstreams].w = w;
	font_stream[font_nstreams].h = h;
	font_stream[font_nstreams].txtbuff = (char*) malloc(n + 1);
	int i = (sizeof(SPRT_8) * n) + 1 * sizeof(DR_TPAGE);
	if (isbg) {
		i += sizeof(TILE);
	}
	font_stream[font_nstreams].pribuff = (char*) malloc(i);
	font_stream[font_nstreams].maxchars = n;
	font_stream[font_nstreams].txtbuff[0] = 0x0;
	font_stream[font_nstreams].txtnext = font_stream[font_nstreams].txtbuff;
	font_stream[font_nstreams].bg = isbg;
	font_stream[font_nstreams].shadow = shadow;
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
	size_t n = strlen(font_stream[id].txtbuff);
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
		id = font_nstreams - 1;
	}
	const Texture* tex_ref = font_current;
	int stream_x = font_stream[id].x;
	int stream_y = font_stream[id].y;
	const char* text = font_stream[id].txtbuff;
	char* primitive = font_stream[id].pribuff;
	// Create TPage primitive
	DR_TPAGE* texture_page = (DR_TPAGE*)primitive;
	setDrawTPage(
		texture_page,
		0,
		0,
		tex_ref->tpage
	);
	// Increment the xcoord to target the shadow half of the
	// texture at an offset of 128
	const u16 texture_offset = font_stream[id].shadow ? 128 : 0;
	SPRT_8* sprite;
	// Create a black rectangle background when enabled
	if (font_stream[id].bg) {
		primitive += sizeof(DR_TPAGE);
		TILE* tile = (TILE*)primitive;
		setTile(tile);
		if (font_stream[id].bg == 2) {
			setSemiTrans(tile, 1);
		}
		setXY0(tile, font_stream[id].x, font_stream[id].y);
		setWH(tile, font_stream[id].w, font_stream[id].h);
		setRGB0(tile, 0, 0, 0);
		setaddr(texture_page, tile);
		primitive = (char*)tile;
		sprite = (SPRT_8*)(primitive + sizeof(TILE));
	} else {
		sprite = (SPRT_8*)(primitive + sizeof(DR_TPAGE));
	}
	// Create the sprite primitives
	char cc;
	while ((cc = *text) != 0) {
		if (cc == '\n' || stream_x - font_stream[id].x > font_stream[id].w - FONT_CHARACTER_SPRITE_WIDTH) {
			stream_x = font_stream[id].x;
			stream_y += FONT_CHARACTER_SPRITE_HEIGHT;
			if (cc == '\n') {
				text++;
			}
			continue;
		}
		if (stream_y - font_stream[id].y > font_stream[id].h - FONT_CHARACTER_SPRITE_HEIGHT) {
			break;
		}
		const int c = (u8) cc;
		if (c > 0 && c <= UINT8_MAX) {
			setSprt8(sprite);
			setShadeTex(sprite, 1);
			setSemiTrans(sprite, 0);
			setXY0(sprite, stream_x, stream_y);
			setUV0(
				sprite,
				((c % FONT_SPRITE_WIDTH) * FONT_CHARACTER_SPRITE_WIDTH) + texture_offset,
				((c / FONT_SPRITE_HEIGHT) * FONT_CHARACTER_SPRITE_HEIGHT) + FONT_SPRITE_V_OFFSET
			);
			sprite->clut = tex_ref->clut;
			setaddr(primitive, sprite);
			primitive = (char*)sprite;
			sprite++;
		}
		stream_x += FONT_CHARACTER_SPRITE_WIDTH;
		text++;
	}
	// Set a terminator value to the last primitive
	termPrim(primitive);
	// Draw the primitives
	DrawSync(0);
	DrawOTag((u32*) font_stream[id].pribuff);
	DrawSync(0);
	font_stream[id].txtnext = font_stream[id].txtbuff;
	font_stream[id].txtbuff[0] = 0;
	return (void*) font_stream[id].pribuff;
}

void* fontSort(u32* ordering_table,
			   void* primitive,
			   const int x,
			   const int y,
			   const bool shadow,
			   const char *text) {
	_sdk_validate_args(ordering_table && primitive, 0);
	const Texture* tex_ref = font_current;
	const u16 texture_offset = shadow ? 128 : 0;
	SPRT_8* sprite = (SPRT_8*)primitive;
	int stream_x = x;
	int stream_y = y;
	char cc;
	while ((cc = *text) != 0) {
		if (cc == '\n') {
			stream_x = x;
			stream_y += FONT_CHARACTER_SPRITE_HEIGHT;
			text++;
			continue;
		}
		const int i = (u8) cc;
		if (i > 0 && i < UINT8_MAX) {
			setSprt8(sprite);
			setShadeTex(sprite, 1);
			setSemiTrans(sprite, 1);
			setXY0(sprite, stream_x, stream_y);
			setUV0(
				sprite,
				((i % FONT_SPRITE_WIDTH) * FONT_CHARACTER_SPRITE_WIDTH) + texture_offset,
				((i / FONT_SPRITE_HEIGHT) * FONT_CHARACTER_SPRITE_HEIGHT) + FONT_SPRITE_V_OFFSET
			);
			sprite->clut = tex_ref->clut;
			addPrim(ordering_table, sprite);
			sprite++;
		}
		stream_x += FONT_CHARACTER_SPRITE_WIDTH;
		text++;
	}
	primitive = (char*)sprite;
	DR_TPAGE* texture_page = (DR_TPAGE*) primitive;
	setDrawTPage(
		texture_page,
		0,
		0,
		tex_ref->tpage
	);
	setTPageSemiTrans(texture_page, 1);
	// texture_page->code[0] &= ~((u32) 96);
	// texture_page->code[0] |= 32;
	addPrim(ordering_table, primitive);
	primitive += sizeof(DR_TPAGE);
	return (void *) primitive;
}
