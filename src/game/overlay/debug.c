#include "debug.h"

#include "../blocks/block.h"
#include "../../logging/logging.h"

void drawLeftDebugText(const Stats* stats, const Camera* camera, MAYBE_UNUSED const World* world) {
#if isDebugFlagEnabled(OVERLAY_FPS)
    FntPrint(0, "FT=%dms FPS=%d TPS=%d\n", stats->frame_diff_ms, stats->fps, stats->tps);
#endif
#if isDebugFlagEnabled(OVERLAY_POS)
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
#if isDebugFlagEnabled(OVERLAY_DIR)
    FntPrint(
        0,
        "RX=%d RY=%d\n",
        camera->rotation.vx >> FIXED_POINT_SHIFT,
        camera->rotation.vy >> FIXED_POINT_SHIFT
    );
    FntPrint(
        0,
        "DX=%d DY=%d DZ=%d\n",
        VEC_LAYOUT(camera->direction)
    );
#endif
#if isDebugFlagEnabled(OVERLAY_FACING)
    char facing[7];
#define setFacing(dir, sign, axis) \
    facing[0] = (dir); \
    facing[1] = ' '; \
    facing[2] = '('; \
    facing[3] = (sign); \
    facing[4] = (axis); \
    facing[5] = ')'; \
    facing[6] = 0
    switch (faceDirectionClosestNormal(camera->direction)) {
        // NOTE: Up and down are swapping since the above function
        //       returns a value based world position, not camera
        //       position. So Y is inverted.
        case FACE_DIR_DOWN: setFacing('U', '+', 'Y'); break;
        case FACE_DIR_UP: setFacing('D', '-', 'Y'); break;
        case FACE_DIR_LEFT: setFacing('L', '-', 'X'); break;
        case FACE_DIR_RIGHT: setFacing('R', '+', 'X'); break;
        case FACE_DIR_BACK: setFacing('B', '+', 'Z'); break;
        case FACE_DIR_FRONT: setFacing('F', '-', 'Z'); break;
        default: errorAbort("Unhandled facing direction"); return;
    }
    FntPrint(
        0,
        "FACING=%s\n",
        facing
    );
#undef setFacing
#endif
#if isDebugFlagEnabled(OVERLAY_WORLD)
    char* weather;
    if (world->weather.storming) {
        weather = "storming";
    } else if (world->weather.raining) {
        weather = "raining";
    } else {
        weather = "clear";
    }
    FntPrint(
        0,
        "Day=%d Time=%d\nWeather=%s\n",
        world->day_count,
        world->time_ticks,
        weather
    );
#endif
}

void drawRightDebugText(MAYBE_UNUSED const Stats* stats) {
#if isDebugFlagEnabled(OVERLAY_MEM)
    HeapUsage* usage = {0};
    GetHeapUsage(usage);
    char* suffix;
    u32 whole;
    u32 frac;
    humanSize(usage->total, &suffix, &whole, &frac);
    FntPrint(
        1,
        "Memory: %d.%d%s\n",
        whole, frac, suffix
    );
    char* suffix2;
    u32 whole2;
    u32 frac2;
    humanSize(usage->heap, &suffix, &whole, &frac);
    humanSize(usage->alloc, &suffix2, &whole2, &frac2);
    FntPrint(
        1,
        "Heap: %d.%d%s / %d.%d%s\n",
        whole2, frac2, suffix2,
        whole, frac, suffix
    );
    humanSize(usage->alloc_max, &suffix, &whole, &frac);
    FntPrint(
        1,
        "Max Heap: %d.%d%s\n",
        whole, frac, suffix
    );
    humanSize(usage->stack, &suffix, &whole, &frac);
    FntPrint(
        1,
        "Stack: %d.%d%s\n",
        whole, frac, suffix
    );
#endif
}

void debugOverlayDraw(const Stats* stats, const Camera* camera, const World* world) {
    drawLeftDebugText(stats, camera, world);
    drawRightDebugText(stats);
}
