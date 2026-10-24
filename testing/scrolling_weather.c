#include <psxgpu.h>
#include <psxgte.h>
#include <psxcd.h>
#include <inline_c.h>

#include "logging/logging.h"
#include "core/input/input.h"
#include "math/math_utils.h"
#include "render/render_context.h"
#include "resources/asset_indices.h"
#include "resources/assets.h"
#include "resources/texture.h"
#include "util/inttypes.h"
#include "render/font.h"

int main() {
    RenderContext ctx = (RenderContext) {
        .active = 0,
        .db = {0},
        .primitive = NULL,
        .camera = NULL
    };
    memset(&ctx, '\0', sizeof(RenderContext));
    initRenderContext(&ctx);
    inputInit(&input);
    CdInit();
    assetsLoad();
    fontLoad();
    FntLoad(960, 256);
    FntOpen(0, 8, 320, 216, 0, 160);
    const u32 w = 32;
    const u32 h = 120;
    assert(h <= 120 && "Height must be at in range (0, 120]");
    RECT tex_window = textureWindowCreate(256, 256, 0, 0);
    DEBUG_LOG(
        "Window [Mask X: %d] [Mask Y: %d] [Offset X: %d] [Offset Y: %d]\n",
        tex_window.w,
        tex_window.h,
        tex_window.x,
        tex_window.y
    );
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__WEATHER];
    u32 ticks = 0;
    i32 shift = 0;
    while (true) {
        if (ticks % 2 == 0) {
            if (isPressed(input.pad, PAD_UP)) {
                shift = (shift + 1) % 240;
            } else if (isPressed(input.pad, PAD_DOWN)) {
                shift = positiveModulo(shift - 1, 240);
            }
        }
        const u32 u = 64 * (shift >= (240 >> 1));
        const u32 v = 240 - h - (shift % (240 >> 1));
        FntPrint(0, "Shift: %d\n", shift);
        FntPrint(0, "UV: %d, %d\n", u, v);
        FntFlush(0);
        renderClearConstraints(&ctx);
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(&ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(pol4, CENTRE_X - 16, CENTRE_Y - 32, w, h);
        setUVWH(pol4, u, v, w, h);
        setRGB0(pol4, 0x7F, 0x7F, 0x7F);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        u32* ot_object = allocateOrderingTable(&ctx, 1);
        addPrim(ot_object, pol4);
        // DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(&ctx, sizeof(DR_TWIN));
        // setTexWindow(ptwin, &tex_window);
        // addPrim(ot_object, ptwin);
        // renderClearConstraints(&ctx);
        swapBuffers(&ctx);
        ticks++;
    }
    return 0;
}
