#include "background.h"

#include <noise.h>
#include <primitive.h>

#include "../resources/assets.h"
#include "../resources/asset_indices.h"
#include "../resources/texture.h"
#include "psxgpu.h"

#ifndef BLOCK_TEXTURE_SIZE
#define BLOCK_TEXTURE_SIZE 16
#endif

void drawBackgroundHalf(RenderContext* ctx,
                        const u32* ot_entry,
                        const u8 u,
                        const u8 v,
                        const u16 x) {
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    // Set full screen vertex positions
    setXY4(
        pol4,
        x, 0,
        CENTRE_X + x, 0,
        x, SCREEN_YRES,
        CENTRE_X + x, SCREEN_YRES
    );
    // Mid point grey as mask for additive texturing
    setRGB0(pol4, 0x28, 0x28, 0x28);
    // Set texture coords and dimensions
    setUVWH(
        pol4,
        u,
        v,
        CENTRE_X,
        SCREEN_YRES
    );
    // Bind texture page and colour look-up-table
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    // Sort primitive to the ordering table
    addPrim(ot_entry, pol4);
}

void backgroundDraw(RenderContext* ctx,
                    const u32* ot_object,
                    const u8 u,
                    const u8 v) {
    // BUG: For some reason using a polygon with width of the screen doesn't wrap textures
    //      properly even with texture windowing.
    // Left half of the screen
    drawBackgroundHalf(ctx, ot_object, u, v, 0);
    // Right half of the screen
    drawBackgroundHalf(ctx, ot_object, u, v, CENTRE_X);
    const TextureWindow tex_window = textureWindowCreate(16, 16, u, v);
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    addPrim(ot_object, ptwin);
    renderClearConstraints(ctx);
}
