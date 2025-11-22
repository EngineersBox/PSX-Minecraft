#include "background.h"

#include <psxgpu.h>

#include "../resources/assets.h"
#include "../resources/asset_indices.h"
#include "../resources/texture.h"

#ifndef BLOCK_TEXTURE_SIZE
#define BLOCK_TEXTURE_SIZE 16
#endif

static void drawBackgroundHalf(RenderContext* ctx,
                               const u32* ot_entry,
                               const u8 u,
                               const u8 v,
                               const u16 x) {
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setXYWH(
        pol4,
        x,
        0,
        SCREEN_XRES >> 1,
        SCREEN_YRES
    );
    setRGB0(pol4, 0x28, 0x28, 0x28);
    setUVWH(
        pol4,
        u,
        v,
        SCREEN_XRES >> 1,
        SCREEN_YRES
    );
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    addPrim(ot_entry, pol4);
}

void backgroundDraw(RenderContext* ctx,
                    u32* ot_object,
                    const u8 u,
                    const u8 v) {
    // Left half of the screen
    drawBackgroundHalf(ctx, ot_object, u, v, 0);
    // Right half of the screen
    drawBackgroundHalf(ctx, ot_object, u, v, CENTRE_X);
    const TextureWindow tex_window = textureWindowCreate(16, 16, u, v);
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_object, ptwin);
    renderClearConstraintsEntry(ctx, ot_object);
}
