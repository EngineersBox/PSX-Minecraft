/*#include "core/engine.h"*/
/*#include "core/app_logic.h"*/
/*#include "game/minecraft.h"*/
/**/
/*static AppLogic app_logic;*/
/*static Minecraft* minecraft;*/
/**/
/*int main() {*/
/*    app_logic = DYN_LIT(Minecraft, AppLogic, {});*/
/*    minecraft = (Minecraft*) &app_logic;*/
/*    Engine engine = (Engine) {*/
/*        .app_logic =  &app_logic,*/
/*        .target_fps = 60,*/
/*        .target_tps = 20*/
/*    };*/
/*    engineInit(&engine, NULL);*/
/*    engineRun(&engine);*/
/*    return 0;*/
/*}*/

#include <psxgpu.h>
#include <psxgte.h>
#include <psxcd.h>
#include <inline_c.h>

#include "logging/logging.h"
#include "core/input/input.h"
#include "render/render_context.h"
#include "render/transforms.h"
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
    const u32 u = 11 * 16;
    const u32 v = 2 * 16;
    const u32 w = 32;
    const u32 h = 32;
    RECT tex_window = textureWindowCreate(16, 16, u, v);
    DEBUG_LOG(
        "Window [Mask X: %d] [Mask Y: %d] [Offset X: %d] [Offset Y: %d]\n",
        tex_window.w,
        tex_window.h,
        tex_window.x,
        tex_window.y
    );
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    int ticks = 0;
    i16* target = &tex_window.w;
    char* selected = "Selected: Mask X\n";
    while (true) {
        if (ticks == 0) {
            if (isPressed(input.pad, PAD_TRIANGLE)) {
                target = &tex_window.w;
                selected = "Selected: Mask X\n";
            } else if (isPressed(input.pad, PAD_CIRCLE)) {
                target = &tex_window.h;
                selected = "Selected: Mask Y\n";
            } else if (isPressed(input.pad, PAD_CROSS)) {
                target = &tex_window.x;
                selected = "Selected: Offset X\n";
            } else if (isPressed(input.pad, PAD_SQUARE)) {
                target = &tex_window.y;
                selected = "Selected: Offset Y\n";
            }
            if (isPressed(input.pad, PAD_UP)) {
                if (*target < 0b11111) (*target)++;
            } else if (isPressed(input.pad, PAD_DOWN)) {
                if (*target > 0b00000) (*target)--;
            }
        }
        FntPrint(0, selected);
        FntPrint(0, "Mask X: %d = 0b" INT8_BIN_PATTERN "\n", tex_window.w, INT8_BIN_LAYOUT(tex_window.w));
        FntPrint(0, "Mask Y: %d = 0b" INT8_BIN_PATTERN "\n", tex_window.h, INT8_BIN_LAYOUT(tex_window.h));
        FntPrint(0, "Offset X: %d = 0b" INT8_BIN_PATTERN "\n", tex_window.x, INT8_BIN_LAYOUT(tex_window.x));
        FntPrint(0, "Offset Y: %d = 0b" INT8_BIN_PATTERN "\n", tex_window.y, INT8_BIN_LAYOUT(tex_window.y));
        FntFlush(0);
        renderClearConstraints(&ctx);
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(&ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(pol4, CENTRE_X - 16, CENTRE_Y - 16, 32, 32);
        setUVWH(pol4, u, v, w, h);
        setRGB0(pol4, 0x7F, 0x7F, 0x7F);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        u32* ot_object = allocateOrderingTable(&ctx, 1);
        addPrim(ot_object, pol4);
        DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(&ctx, sizeof(DR_TWIN));
        setTexWindow(ptwin, &tex_window);
        addPrim(ot_object, ptwin);
        renderClearConstraints(&ctx);
        swapBuffers(&ctx);
        ticks = (ticks + 1) % 5;
    }
    return 0;
}
