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
#include <inline_c.h>

#include "logging/logging.h"
#include "psxcd.h"
#include "render/render_context.h"
#include "render/transforms.h"
#include "resources/asset_indices.h"
#include "resources/assets.h"
#include "resources/texture.h"

int main() {
    RenderContext ctx = (RenderContext) {
        .active = 0,
        .db = {},
        .primitive = NULL,
        .camera = NULL
    };
    initRenderContext(&ctx);
    CdInit();
    assetsLoad();
    const u32 u = 11 * 16;
    const u32 v = 2 * 16;
    const u32 w = 32;
    const u32 h = 32;
    RECT tex_window = (RECT) {
        .w = 0b10110,//textureWindowMaskBlock(u),
        .h = 0b00110,//textureWindowMaskBlock(v),
        .x = 0b11110,//textureWindowOffsetBlock(u),
        .y = 0b00000,//textureWindowOffsetBlock(v)
    };
    DEBUG_LOG(
        "Window [Mask X: %d] [Mask Y: %d] [Offset X: %d] [Offset Y: %d]\n",
        tex_window.w,
        tex_window.h,
        tex_window.x,
        tex_window.y
    );
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    while (true) {
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(&ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(pol4, CENTRE_X - 16, CENTRE_Y - 16, 32, 32);
        setUVWH(pol4, u, v, w, h);
        setRGB0(pol4, 0x7F, 0x7F, 0x7F);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        u32* ot_object = allocateOrderingTable(&ctx, 0);
        addPrim(ot_object, pol4);
        DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(&ctx, sizeof(DR_TWIN));
        setTexWindow(ptwin, &tex_window);
        addPrim(ot_object, ptwin);
        swapBuffers(&ctx);
    }
    return 0;
}
