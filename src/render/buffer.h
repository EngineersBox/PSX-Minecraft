#ifndef _PSXMC_RENDER_BUFFER
#define _PSXMC_RENDER_BUFFER

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
	uint8_t* next_packet;
	int active_buffer;
} RenderContext;

void setup_context(RenderContext* ctx, int w, int h, int r, int g, int b);
void flip_buffers(RenderContext* ctx);

#endif // _PSXMC_RENDER_BUFFER