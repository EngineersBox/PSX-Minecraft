#include "crosshair.h"

#include <psxgpu.h>

#include "../resources/assets.h"
#include "../resources/asset_indices.h"
#include "../structure/primitive/primitive.h"

void crosshairDraw(RenderContext* ctx) {
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setXYWH(
        pol4,
        CENTRE_X - 8,
        CENTRE_Y - 8,
        16,
        16
    );
    setUVWH(
        pol4,
        182,
        24,
        16,
        16
    );
    setRGB0(pol4, 0xff, 0xff, 0xff);
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI_PART];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    polyFT4Render(pol4, 0, ctx);
}
