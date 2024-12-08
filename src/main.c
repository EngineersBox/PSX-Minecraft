#include "core/engine.h"
#include "core/app_logic.h"
#include "game/minecraft.h"

static AppLogic app_logic;
static Minecraft* minecraft;

int main() {
    app_logic = DYN_LIT(Minecraft, AppLogic, {});
    minecraft = (Minecraft*) &app_logic;
    Engine engine = (Engine) {
        .app_logic =  &app_logic,
        .target_fps = 60,
        .target_tps = 20
    };
    engineInit(&engine, NULL);
    engineRun(&engine);
    return 0;
}

/*#include <psxgpu.h>*/
/*#include <psxgte.h>*/
/*#include <inline_c.h>*/
/**/
/*#include "psxcd.h"*/
/*#include "render/render_context.h"*/
/*#include "render/transforms.h"*/
/*#include "resources/asset_indices.h"*/
/*#include "resources/assets.h"*/
/**/
/*int main() {*/
/*    RenderContext ctx = (RenderContext) {*/
/*        .active = 0,*/
/*        .db = {},*/
/*        .primitive = NULL,*/
/*        .camera = NULL*/
/*    };*/
/*    initRenderContext(&ctx);*/
/*    CdInit();*/
/*    assetsLoad();*/
/*    const u32 u = 11 * 16;*/
/*    const u32 v = 2 * 16;*/
/*    const u32 w = 16;*/
/*    const u32 h = 16;*/
/*    RECT tex_window = (RECT) {*/
/*        0b0010,*/
/*        0b0010,*/
/*        0b0000,*/
/*        0b0000*/
/*    };*/
/*    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];*/
/*    while (true) {*/
/*        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(&ctx, sizeof(POLY_FT4));*/
/*        setPolyFT4(pol4);*/
/*        setXYWH(pol4, CENTRE_X - 16, CENTRE_Y - 16, 16, 16);*/
/*        setUVWH(pol4, u, v, w, h);*/
/*        setRGB0(pol4, 0xFF, 0xFF, 0xFF);*/
/*        pol4->tpage = texture->tpage;*/
/*        pol4->clut = texture->clut;*/
/*        u32* ot_object = allocateOrderingTable(&ctx, 0);*/
/*        addPrim(ot_object, pol4);*/
/*        DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(&ctx, sizeof(DR_TWIN));*/
/*        setTexWindow(ptwin, &tex_window);*/
/*        addPrim(ot_object, ptwin);*/
/*        renderClearConstraints(&ctx);*/
/*        swapBuffers(&ctx);*/
/*    }*/
/*    return 0;*/
/*}*/
