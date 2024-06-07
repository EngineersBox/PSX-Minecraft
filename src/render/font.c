#include "font.h"

#include <asset_indices.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "../logging/logging.h"
#include "../resources/assets.h"

typedef struct {
	char* txtbuff;
	char* txtnext;
	char* pribuff;
	i16	x, y;
	i16	w, h;
	int	bg, maxchars;
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

void fontLoad() {
	if (!assets_loaded) {
		errorAbort("[ERROR] Cannot load font, assets have not been loaded\n");
	}
	font_current = &textures[ASSET_TEXTURES_FONT_INDEX];
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
			    const int isbg,
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
		id = font_nstreams - 1;
	}
	int stream_x = font_stream[id].x;
	int stream_y = font_stream[id].y;
	const char* text = font_stream[id].txtbuff;
	char* primitive = font_stream[id].pribuff;
	// Create TPage primitive
	DR_TPAGE* texture_page = (DR_TPAGE*)primitive;
	setDrawTPage(texture_page, 1, 0, font_current->tpage);
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
		sprite = (SPRT_8*)(primitive+sizeof(TILE));
	} else {
		sprite = (SPRT_8*)(primitive+sizeof(DR_TPAGE));
	}
	// Create the sprite primitives
	while (*text != 0) {
		if (*text == '\n' || stream_x - font_stream[id].x > font_stream[id].w - FONT_CHARACTER_SPRITE_WIDTH) {
			stream_x = font_stream[id].x;
			stream_y += FONT_CHARACTER_SPRITE_HEIGHT;
			if (*text == '\n') {
				text++;
			}
			continue;
		}
		if (stream_y - font_stream[id].y > font_stream[id].h - FONT_CHARACTER_SPRITE_HEIGHT) {
			break;
		}
		const int c = (u8) *text;
		if (c > 0 && c <= UINT8_MAX) {
			setSprt8(sprite);
			setShadeTex(sprite, 1);
			setSemiTrans(sprite, 1);
			setXY0(sprite, stream_x, stream_y);
			setUV0(
				sprite,
				(c % FONT_SPRITE_WIDTH) * FONT_CHARACTER_SPRITE_WIDTH,
				(c / FONT_SPRITE_HEIGHT) * FONT_CHARACTER_SPRITE_HEIGHT
			);
			sprite->clut = font_current->clut;
			setaddr(primitive, sprite);
			primitive = (char*)sprite;
			sprite++;
		}
		stream_x += FONT_CHARACTER_SPRITE_WIDTH;
		text++;
	}
	// NOTE: setaddr sets the address of the next primitive to the target primitive. Essentially,
	//       the ordering table is a unidirectional linked list
	// setaddr(primitive, sprite);
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
			   const char *text) {
	_sdk_validate_args(ordering_table && primitive, 0);
	SPRT_8* sprite = (SPRT_8*)primitive;
	int stream_x = x;
	int stream_y = y;
	while (*text != 0) {
		if (*text == '\n') {
			stream_x = x;
			stream_y += FONT_CHARACTER_SPRITE_HEIGHT;
			text++;
			continue;
		}
		const int i = (u8) *text;
		if (i > 0 && i < UINT8_MAX) {
			setSprt8(sprite);
			setShadeTex(sprite, 1);
			setSemiTrans(sprite, 1);
			setXY0(sprite, stream_x, stream_y);
			setUV0(
				sprite,
				(i % FONT_SPRITE_WIDTH) * FONT_CHARACTER_SPRITE_WIDTH,
				(i / FONT_SPRITE_HEIGHT) * FONT_CHARACTER_SPRITE_HEIGHT
			);
			sprite->clut = font_current->clut;
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
		1,
		0,
		font_current->tpage
	);
	addPrim(ordering_table, primitive);
	primitive += sizeof(DR_TPAGE);
	return (void *) primitive;
}