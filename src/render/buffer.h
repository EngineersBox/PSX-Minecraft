#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

/* OT and Packet Buffer sizes */
#define OT_LENGTH 256
#define BUFFER_LENGTH 1024

/* Screen resolution */
#define SCREEN_XRES	320
#define SCREEN_YRES	240

/* Screen center position */
#define CENTERX	SCREEN_XRES >> 1
#define CENTERY	SCREEN_YRES >> 1

/* Double buffer structure */
typedef struct {
	DISPENV	display_env;
	DRAWENV	draw_env;
	uint32_t ordering_table[OT_LENGTH];
	char packet_buffer[BUFFER_LENGTH];
} RenderBuffer;

typedef struct {
	RenderBuffer buffers[2];
	uint8_t      *next_packet;
	int          active_buffer;
} RenderContext;

void setup_context(RenderContext *ctx, int w, int h, int r, int g, int b) {
	// Place the two framebuffers vertically in VRAM.
	SetDefDrawEnv(&(ctx->buffers[0].draw_env), 0, 0, w, h);
	SetDefDispEnv(&(ctx->buffers[0].display_env), 0, 0, w, h);
	SetDefDrawEnv(&(ctx->buffers[1].draw_env), 0, h, w, h);
	SetDefDispEnv(&(ctx->buffers[1].display_env), 0, h, w, h);

	// Set the default background color and enable auto-clearing.
	setRGB0(&(ctx->buffers[0].draw_env), r, g, b);
	setRGB0(&(ctx->buffers[1].draw_env), r, g, b);
	ctx->buffers[0].draw_env.isbg = 1;
	ctx->buffers[1].draw_env.isbg = 1;

	// Initialize the first buffer and clear its OT so that it can be used for
	// drawing.
	ctx->active_buffer = 0;
	ctx->next_packet = ctx->buffers[0].packet_buffer;
	ClearOTagR(ctx->buffers[0].ordering_table, OT_LENGTH);

	// Turn on the video output.
	SetDispMask(1);
}