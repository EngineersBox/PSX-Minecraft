#include "background.h"

#include <psxgpu.h>

#include "../../structure/primitive/primitive.h"
#include "../../util/preprocessor.h"
#include "../../util/memory.h"

INLINE UIBackground* UIBackgroundNew() {
    UIBackground* bg = malloc(sizeof(UIBackground));
    assert(bg != NULL);
    zeroed(bg);
    return bg;
};

void uiBackgroundRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("UIBackground_render");
void UIBackground_render(VSelf, RenderContext* ctx, UNUSED Transforms* transforms) {
    VSELF(UIBackground);
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        pol4,
        self->component.position.vx,
        self->component.position.vy,
        self->component.dimensions.vx,
        self->component.dimensions.vy
    );
    setUVWH(
        pol4,
        self->texture_coords.vx,
        self->texture_coords.vy,
        self->texture_width.vx,
        self->texture_width.vy
    );
    // Mid point grey as mask for additive texturing
    setRGB0(
        pol4,
        self->tint.r,
        self->tint.g,
        self->tint.b
    );
    pol4->tpage = self->texture.tpage;
    pol4->clut = self->texture.clut;
    polyFT4Render(pol4, 1, ctx);
    renderClearConstraintsIndex(ctx, 1);
}
