#include "debug.h"

#include <psxgpu.h>
#include <stdlib.h>

#include "../math/math_utils.h"
#include "../primitive/primitive.h"
#include "../game/blocks/block.h"
#include "../util/memory.h"

DEFINE_CIRCULAR_BUFFER(ordering_table_usage, SAMPLE_WINDOW_SIZE);
#define OT_DATA_POINT_PER_PIXEL (ORDERING_TABLE_LENGTH / SAMPLE_MAX_VALUE)

DEFINE_CIRCULAR_BUFFER(packet_buffer_usage, SAMPLE_WINDOW_SIZE);
#define PB_DATA_POINT_PER_PIXEL (PACKET_BUFFER_LENGTH / SAMPLE_MAX_VALUE)

#define SAMPLE_RGB_INCREMENT (255 / SAMPLE_MAX_VALUE)

void renderUsageGraph(RenderContext* ctx, 
                      const CircularBuffer* buffer,
                      const u16 base_screen_x,
                      const u16 base_screen_y) {
    for (int i = 0; i < buffer->count; i++) {
        const int8_t usage = buffer->buffer[(buffer->tail + i) % buffer->maxlen];
        LINE_G2* line = (LINE_G2*) allocatePrimitive(ctx, sizeof(LINE_G2));
        setXY2(
            line,
            base_screen_x + i, base_screen_y,
            base_screen_x + i, base_screen_y - usage - 1
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

void renderBlackBackground(RenderContext* ctx,
                           const u16 base_screen_x,
                           const u16 base_screen_y,
                           const u16 width,
                           const u16 height) {
    TILE* tile = (TILE*) allocatePrimitive(ctx, sizeof(TILE));
    setXY0(tile, base_screen_x, base_screen_y - height);
    setWH(tile, width, height);
    setRGB0(tile, 1, 1, 1);
    tileRender(tile, 1, ctx);
}

void debugDrawPBUsageGraph(RenderContext* ctx, const u16 base_screen_x, const u16 base_screen_y) {
    static int sampledPB = 0;
    if (sampledPB < SAMPLE_RATE) {
        sampledPB++;
        renderBlackBackground(
            ctx,
            base_screen_x,
            base_screen_y,
            SAMPLE_WINDOW_SIZE,
            SAMPLE_MAX_VALUE
        );
        renderUsageGraph(ctx, &packet_buffer_usage, base_screen_x, base_screen_y);
        return;
    }
    sampledPB = 0;
    ptrdiff_t used = (uintptr_t) ctx->primitive - (uintptr_t) ctx->db[ctx->active].packet_buffer;
    used /= sizeof(char);
    used /= PB_DATA_POINT_PER_PIXEL;
    circularBufferPush(&packet_buffer_usage, used);
    renderBlackBackground(
        ctx,
        base_screen_x,
        base_screen_y,
        SAMPLE_WINDOW_SIZE,
        SAMPLE_MAX_VALUE
    );
    renderUsageGraph(ctx, &packet_buffer_usage, base_screen_x, base_screen_y);
}

#define isOverlayEnabled(suffix) (isDebugEnabled() && defined(PSXMC_DEBUG_OVERLAY_##suffix) && PSXMC_DEBUG_OVERLAY_##suffix)

void drawLeftDebugText(const Stats* stats, const Camera* camera) {
#if isOverlayEnabled(FPS)
    FntPrint(0, "FT=%dms FPS=%d TPS=%d\n", stats->frame_diff_ms, stats->fps, stats->tps);
#endif
#if isOverlayEnabled(POS)
    const i32 x = camera->position.vx / BLOCK_SIZE;
    const i32 y_down = camera->position.vy / BLOCK_SIZE;
    const i32 y_up = -camera->position.vy / BLOCK_SIZE;
    const i32 z = camera->position.vz / BLOCK_SIZE;
    #define fracToFloat(frac) ((u32)(100000 * ((frac) / 4096.0)))
    FntPrint(
        0,
        ""
        "X=%d.%05d\n"
        "Y=%d.%05d (D)\n"
        "Y=%d.%05d (U)\n"
        "Z=%d.%05d\n",
        fixedGetWhole(x), fracToFloat(fixedGetFractional(x)),
        fixedGetWhole(y_down), fracToFloat(fixedGetFractional(y_down)),
        fixedGetWhole(y_up), fracToFloat(fixedGetFractional(y_up)),
        fixedGetWhole(z), fracToFloat(fixedGetFractional(z))
    );
    #undef fracToFloat
#endif
#if (isOverlayEnabled(DIR) || isOverlayEnabled(FACING))
    const VECTOR direction = rotationToDirection(&camera->rotation);
#endif
#if isOverlayEnabled(DIR)
    FntPrint(
        0,
        "RX=%d RY=%d\n",
        camera->rotation.vx >> FIXED_POINT_SHIFT,
        camera->rotation.vy >> FIXED_POINT_SHIFT
    );
    FntPrint(
        0,
        "DX=%d DY=%d DZ=%d\n",
        VEC_LAYOUT(direction)
    );
#endif
#if isOverlayEnabled(FACING)
    char facing;
    switch (faceDirectionClosestNormal(direction)) {
        // NOTE: Up and down are swapping since the above function
        //       returns a value based world position, not camera
        //       position. So Y is inverted.
        case FACE_DIR_DOWN: facing = 'U'; break;
        case FACE_DIR_UP: facing = 'D'; break;
        case FACE_DIR_LEFT: facing = 'L'; break;
        case FACE_DIR_RIGHT: facing = 'R'; break;
        case FACE_DIR_BACK: facing = 'B'; break;
        case FACE_DIR_FRONT: facing = 'F'; break;
    }
    FntPrint(
        0,
        "FACING=%c\n",
        facing
    );
#endif
}

void drawRightDebugText(const Stats* stats) {
#if isOverlayEnabled(MEM)
    HeapUsage* usage = {0};
    GetHeapUsage(usage);
    char* suffix;
    u32 whole;
    u32 frac;
    humanSize(usage->total, &suffix, &whole, &frac);
    FntPrint(
        0,
        "Memory: %d.%d%s\n",
        whole, frac, suffix
    );
    char* suffix2;
    u32 whole2;
    u32 frac2;
    humanSize(usage->heap, &suffix, &whole, &frac);
    humanSize(usage->alloc, &suffix2, &whole2, &frac2);
    FntPrint(
        0,
        "Heap: %d.%d%s / %d.%d%s\n",
        whole2, frac2, suffix2,
        whole, frac, suffix
    );
    humanSize(usage->alloc_max, &suffix, &whole, &frac);
    FntPrint(
        0,
        "Max Heap: %d.%d%s\n",
        whole, frac, suffix
    );
    humanSize(usage->stack, &suffix, &whole, &frac);
    FntPrint(
        0,
        "Stack: %d.%d%s\n",
        whole, frac, suffix
    );
#endif
}

void drawDebugText(const Stats* stats, const Camera* camera) {
    drawLeftDebugText(stats, camera);
    drawRightDebugText(stats);
}
#undef isOverlayEnabled
