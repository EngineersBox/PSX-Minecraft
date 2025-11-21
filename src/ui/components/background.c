#include "background.h"

#include <psxgpu.h>

#include "../../structure/primitive/primitive.h"
#include "../../util/preprocessor.h"

UIBackground* uiBackgroundNew(const Texture* texture,
                              const DVECTOR position,
                              const DVECTOR dimensions,
                              const DVECTOR texture_coords,
                              const DVECTOR texture_dimensions,
                              const CVECTOR tint,
                              u8 ot_entry_index) {
    UIBackground* bg = malloc(sizeof(UIBackground));
    assert(bg != NULL);
    bg->component.position = position;
    bg->component.dimensions = dimensions;
    bg->texture = texture;
    bg->texture_coords = texture_coords;
    bg->texture_dimensions = texture_dimensions;
    bg->tint = tint;
    bg->ot_entry_index = ot_entry_index;
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
        self->texture_dimensions.vx,
        self->texture_dimensions.vy
    );
    // Mid point grey as mask for additive texturing
    setRGB0(
        pol4,
        self->tint.r,
        self->tint.g,
        self->tint.b
    );
    pol4->tpage = self->texture->tpage;
    pol4->clut = self->texture->clut;
    polyFT4Render(pol4, self->ot_entry_index, ctx);
    renderClearConstraintsIndex(ctx, self->ot_entry_index);
}
