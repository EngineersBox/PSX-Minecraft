#include "debug.h"

#include <math_utils.h>

DEFINE_CIRCULAR_BUFFER(ordering_table_usage, ORDERING_TABLE_LENGTH);
#define OT_DATA_POINT_PER_PIXEL (ORDERING_TABLE_LENGTH / SAMPLE_MAX_VALUE)

DEFINE_CIRCULAR_BUFFER(packet_buffer_usage, PACKET_BUFFER_LENGTH);
#define PB_DATA_POINT_PER_PIXEL (PACKET_BUFFER_LENGTH / SAMPLE_MAX_VALUE)

#define SAMPLE_RGB_INCREMENT (255 / SAMPLE_MAX_VALUE)

int sampledPB = 0;

void debugDrawPBUsageGraph(RenderContext* ctx, uint16_t base_screen_x, uint16_t base_screen_y) {
    if (sampledPB < SAMPLE_RATE) {
        sampledPB++;
        return;
    }
    sampledPB = 0;
    ptrdiff_t used = (uintptr_t) ctx->primitive - (uintptr_t) ctx->db[ctx->active].packet_buffer;
    used <<= FIXED_POINT_SHIFT;
    used /= PB_DATA_POINT_PER_PIXEL;
    circularBufferPush(&packet_buffer_usage, used);
    int x = 0;
    for (int i = packet_buffer_usage.head; i < packet_buffer_usage.tail; i++) {
        const int8_t usage = packet_buffer_usage.buffer[i];
        LINE_G2* line = (LINE_G2*) allocatePrimitive(ctx, sizeof(LINE_G2));
        setXY2(
            line,
            base_screen_x + x, base_screen_y,
            base_screen_x + x, base_screen_y - usage
        );
        setRGB0(
            line,
            0,
            255,
            0
        );
        setRGB1(
            line,
            usage * SAMPLE_RGB_INCREMENT,
            0,
            0
        );
        lineG2Render(line, 0, ctx);
        x++;
    }
}

void debugDrawOTUsageGraph(RenderContext* ctx, uint16_t base_screen_x, uint16_t base_screen_y) {

}