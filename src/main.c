/*
 * PSn00bSDK basic graphics example
 * (C) 2020-2023 Lameguy64, spicyjpeg - MPL licensed
 *
 * A comprehensive "advanced hello world" example showing how to set up the
 * screen with double buffering, draw basic graphics (a bouncing square) and use
 * PSn00bSDK's debug font API to quickly print some text, all while following
 * best practices. This is not necessarily the simplest hello world example and
 * may look daunting at first glance, but it is a good starting point for more
 * complex programs.
 *
 * In order to avoid cluttering the program with global variables (as many Sony
 * SDK examples and other PSn00bSDK examples written before this one do) two
 * custom structures are employed:
 *
 * - a RenderBuffer structure containing the DISPENV and DRAWENV objects that
 *   represent the location of the framebuffer in VRAM, as well as the ordering
 *   table (OT) used to sort GPU commands/primitives by their Z index and the
 *   actual buffer commands will be written to;
 * - a RenderContext structure holding two RenderBuffer instances plus some
 *   variables to keep track of which buffer is currently being drawn and how
 *   much of its primitive buffer has been filled up so far.
 *
 * A C++ version of this example is also available (see examples/hellocpp).
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <psxgpu.h>

#include "render/buffer.h"

void flip_buffers(RenderContext *ctx) {
	// Wait for the GPU to finish drawing, then wait for vblank in order to
	// prevent screen tearing.
	DrawSync(0);
	VSync(0);

	RenderBuffer *draw_buffer = &(ctx->buffers[ctx->active_buffer]);
	RenderBuffer *disp_buffer = &(ctx->buffers[ctx->active_buffer ^ 1]);

	// Display the framebuffer the GPU has just finished drawing and start
	// rendering the display list that was filled up in the main loop.
	PutDispEnv(&(disp_buffer->display_env));
	DrawOTagEnv(&(draw_buffer->ordering_table[OT_LENGTH - 1]), &(draw_buffer->draw_env));

	// Switch over to the next buffer, clear it and reset the packet allocation
	// pointer.
	ctx->active_buffer ^= 1;
	ctx->next_packet = disp_buffer->packet_buffer;
	ClearOTagR(disp_buffer->ordering_table, OT_LENGTH);
}

void *new_primitive(RenderContext *ctx, int z, size_t size) {
	// Place the primitive after all previously allocated primitives, then
	// insert it into the OT and bump the allocation pointer.
	// TODO: This bump allocator sucks. Make it not suck.
	RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);
	uint8_t *prim = ctx->next_packet;

	addPrim(&(buffer->ordering_table[z]), prim);
	ctx->next_packet += size;

	// Make sure we haven't yet run out of space for future primitives.
	assert(ctx->next_packet <= &(buffer->packet_buffer[BUFFER_LENGTH]));

	return (void *) prim;
}

// A simple helper for drawing text using PSn00bSDK's debug font API. Note that
// FntSort() requires the debug font texture to be uploaded to VRAM beforehand
// by calling FntLoad().
void draw_text(RenderContext *ctx, int x, int y, int z, const char *text) {
	RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);

	ctx->next_packet = (uint8_t *) FntSort(&(buffer->ordering_table[z]), ctx->next_packet, x, y, text);

	assert(ctx->next_packet <= &(buffer->packet_buffer[BUFFER_LENGTH]));
}

/* Main */

#define SCREEN_XRES 320
#define SCREEN_YRES 240

int main(int argc, const char **argv) {
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
		TILE *tile = (TILE *) new_primitive(&ctx, 1, sizeof(TILE));

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
