#include "background.h"

#include <noise.h>

#include "../resources/assets.h"
#include "../blocks/block.h"

void backgroundDraw(RenderContext* ctx, const int ot_entry, const int8_t u, const int8_t v) {
    // BUG: For some reason using a polygon with width of the screen doesn't wrap textures
    //      properly even with texture windowing.
    // Left half of the screen
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    // Set full screen vertex positions
    setXY4(
        pol4,
        0, 0,
        SCREEN_XRES / 2, 0,
        0, SCREEN_YRES,
        SCREEN_XRES / 2, SCREEN_YRES
    );
    // Mid point grey as mask for additive texturing
    setRGB0(pol4, 0x80, 0x80, 0x80);
    // Set texture coords and dimensions
    setUVWH(
        pol4,
        u,
        v,
        SCREEN_XRES / 2,
        SCREEN_YRES
    );
    // Bind texture page and colour look-up-table
    const Texture* texture = &textures[TERRAIN_TEXTURES];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    // Sort primitive to the ordering table
    uint32_t* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, pol4);
    // Right half of the screen
    pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    // Set full screen vertex positions
    setXY4(
        pol4,
        SCREEN_XRES / 2, 0,
        SCREEN_XRES, 0,
        SCREEN_XRES / 2, SCREEN_YRES,
        SCREEN_XRES, SCREEN_YRES
    );
    // Mid point grey as mask for additive texturing
    setRGB0(pol4, 0x80, 0x80, 0x80);
    // Set texture coords and dimensions
    setUVWH(
        pol4,
        u,
        v,
        SCREEN_XRES / 2,
        SCREEN_YRES
    );
    // Bind texture page and colour look-up-table
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    // Sort primitive to the ordering table
    ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, pol4);
    // Advance to make another primitive
    // Bind a texture window to ensure wrapping across merged block face primitives
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    const RECT tex_window = (RECT){
        .x = u >> 3,
        .y = v >> 3,
        .w = BLOCK_TEXTURE_SIZE >> 3,
        .h = BLOCK_TEXTURE_SIZE >> 3
    };
    printf(
        "Window: (%d,%d,%d,%d)\n",
        tex_window.x,
        tex_window.y,
        tex_window.w,
        tex_window.h
    );
    setTexWindow(ptwin, &tex_window);
    ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, ptwin);
    renderClearConstraints(ctx);
}