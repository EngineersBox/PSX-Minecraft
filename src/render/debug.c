#include "debug.h"

#include "../math/math_utils.h"
#include "../primitive/primitive.h"
#include "../game/blocks/block.h"

DEFINE_CIRCULAR_BUFFER(ordering_table_usage, SAMPLE_WINDOW_SIZE);
#define OT_DATA_POINT_PER_PIXEL (ORDERING_TABLE_LENGTH / SAMPLE_MAX_VALUE)

DEFINE_CIRCULAR_BUFFER(packet_buffer_usage, SAMPLE_WINDOW_SIZE);
#define PB_DATA_POINT_PER_PIXEL (PACKET_BUFFER_LENGTH / SAMPLE_MAX_VALUE)

#define SAMPLE_RGB_INCREMENT (255 / SAMPLE_MAX_VALUE)

void renderUsageGraph(RenderContext* ctx,  const CircularBuffer* buffer, const uint16_t base_screen_x,  const uint16_t base_screen_y) {
    for (int i = 0; i < buffer->count; i++) {
        const int8_t usage = buffer->buffer[(buffer->tail + i) % buffer->maxlen];
        LINE_G2* line = (LINE_G2*) allocatePrimitive(ctx, sizeof(LINE_G2));
        setXY2(
            line,
            base_screen_x + i, base_screen_y,
            base_screen_x + i, base_screen_y - usage
        );
        setRGB0(
            line,
            0,
            0xff,
            0
        );
        const uint8_t increment = usage * SAMPLE_RGB_INCREMENT;
        setRGB1(
            line,
            increment,
            0xff - increment,
            0
        );
        lineG2Render(line, 0, ctx);
    }
}

void renderBlackBackground(RenderContext* ctx, const uint16_t base_screen_x, const uint16_t base_screen_y, const uint16_t width, const uint16_t height) {
    POLY_F4* pol4 = (POLY_F4*) allocatePrimitive(ctx, sizeof(POLY_F4));
    setXY4(
        pol4,
        base_screen_x, base_screen_y,
        base_screen_x + width, base_screen_y,
        base_screen_x, base_screen_y - height,
        base_screen_x + width, base_screen_y - height
    );
    setRGB0(pol4, 1, 1, 1);
    polyF4Render(pol4, 1, ctx);
}

void debugDrawPBUsageGraph(RenderContext* ctx, const uint16_t base_screen_x, const uint16_t base_screen_y) {
    static int sampledPB = 0;
    if (sampledPB < SAMPLE_RATE) {
        sampledPB++;
        renderBlackBackground(ctx, base_screen_x, base_screen_y, SAMPLE_WINDOW_SIZE, SAMPLE_MAX_VALUE);
        renderUsageGraph(ctx, &packet_buffer_usage, base_screen_x, base_screen_y);
        return;
    }
    sampledPB = 0;
    ptrdiff_t used = (uintptr_t) ctx->primitive - (uintptr_t) ctx->db[ctx->active].packet_buffer;
    used /= sizeof(char);
    used /= PB_DATA_POINT_PER_PIXEL;
    circularBufferPush(&packet_buffer_usage, used);
    renderBlackBackground(ctx, base_screen_x, base_screen_y, SAMPLE_WINDOW_SIZE, SAMPLE_MAX_VALUE);
    renderUsageGraph(ctx, &packet_buffer_usage, base_screen_x, base_screen_y);
}

#define isOverlayEnabled(suffix) isDebugEnabled() && defined(PSXMC_DEBUG_OVERLAY_##suffix) && PSXMC_DEBUG_OVERLAY_##suffix

void drawDebugText(const Stats* stats, const Camera* camera) {
#if isOverlayEnabled(FPS)
    FntPrint(0, "FPS=%d TPS=%d\n", stats->fps, stats->tps);
#endif
#if isOverlayEnabled(POS)
    const int32_t x = camera->position.vx / BLOCK_SIZE;
    const int32_t y_down = camera->position.vy / BLOCK_SIZE;
    const int32_t y_up = -camera->position.vy / BLOCK_SIZE;
    const int32_t z = camera->position.vz / BLOCK_SIZE;
    #define fracToFloat(frac) ((u32)(100000 * ((frac) / 4096.0)))
    FntPrint(
        0,
        ""
        "X=%d.%05d\n"
        "Y=%d.%05d (DOWN)\n"
        "Y=%d.%05d (UP)\n"
        "Z=%d.%05d\n",
        fixedGetWhole(x), fracToFloat(fixedGetFractional(x)),
        fixedGetWhole(y_down), fracToFloat(fixedGetFractional(y_down)),
        fixedGetWhole(y_up), fracToFloat(fixedGetFractional(y_up)),
        fixedGetWhole(z), fracToFloat(fixedGetFractional(z))
    );
    #undef fracToFloat
#endif
#if isOverlayEnabled(DIR)
    FntPrint(
        0,
        "RX=%d RY=%d\n",
        camera.rotation.vx >> FIXED_POINT_SHIFT,
        camera.rotation.vy >> FIXED_POINT_SHIFT
    );
    const VECTOR direction = rotationToDirection(&camera.rotation);
    FntPrint(
        0,
        "DX=%d DY=%d DZ=%d\n",
        VEC_LAYOUT(direction)
    );
#endif
}

#undef isOverlayEnabled
