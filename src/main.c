

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <psxgpu.h>

#include "render/buffer.h"

void* new_primitive(RenderContext* ctx, int z, size_t size) {
	// Place the primitive after all previously allocated primitives, then
	// insert it into the OT and bump the allocation pointer.
	// TODO: This bump allocator sucks. Make it not suck.
	RenderBuffer* buffer = &(ctx->buffers[ctx->active_buffer]);
	uint8_t* prim = ctx->next_packet;

	addPrim(&(buffer->ordering_table[z]), prim);
	ctx->next_packet += size;

	// Make sure we haven't yet run out of space for future primitives.
	assert(ctx->next_packet <= &(buffer->packet_buffer[BUFFER_LENGTH]));

	return (void*) prim;
}

// A simple helper for drawing text using PSn00bSDK's debug font API. Note that
// FntSort() requires the debug font texture to be uploaded to VRAM beforehand
// by calling FntLoad().
void draw_text(RenderContext* ctx, int x, int y, int z, const char* text) {
	RenderBuffer* buffer = &(ctx->buffers[ctx->active_buffer]);

	ctx->next_packet = (uint8_t *) FntSort(&(buffer->ordering_table[z]), ctx->next_packet, x, y, text);

	assert(ctx->next_packet <= &(buffer->packet_buffer[BUFFER_LENGTH]));
}

/* Main */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

int main(int argc, const char** argv) {
	// Initialize the GPU and load the default font texture provided by
	// PSn00bSDK at (960, 0) in VRAM.
	ResetGraph(0);
	FntLoad(960, 0);

	// Set up our rendering context.
	RenderContext ctx;
	setup_context(&ctx, SCREEN_XRES, SCREEN_YRES, 63, 0, 127);

	int x  = 0, y  = 0;
	int dx = 1, dy = 1;

	for (;;) {
		// Update the position and velocity of the bouncing square.
		if (x < 0 || x > (SCREEN_XRES - 64))
			dx = -dx;
		if (y < 0 || y > (SCREEN_YRES - 64))
			dy = -dy;

		x += dx;
		y += dy;

		// Draw the square by allocating a TILE (i.e. untextured solid color
		// rectangle) primitive at Z = 1.
		TILE* tile = (TILE*) new_primitive(&ctx, 1, sizeof(TILE));

		setTile(tile);
		setXY0 (tile, x, y);
		setWH  (tile, 64, 64);
		setRGB0(tile, 255, 255, 0);

		// Draw some text in front of the square (Z = 0, primitives with higher
		// Z indices are drawn first).
		draw_text(&ctx, 8, 16, 0, "Hello world!");

		flip_buffers(&ctx);
	}

	return 0;
}
