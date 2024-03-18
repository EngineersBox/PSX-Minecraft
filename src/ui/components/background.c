#include "background.h"

#include <psxgpu.h>

#include "../../structure/primitive/primitive.h"

void uiBackgroundAction(VSelf, const DVECTOR* cursor_position, const bool pressed) __attribute__((alias("UIBackground_action")));
void UIBackground_action(VSelf, const DVECTOR* cursor_position, const bool pressed) {
    // Does nothing
}

void uiBackgroundRender(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("UIBackground_render")));
void UIBackground_render(VSelf, RenderContext* ctx, Transforms* transforms) {
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
    const Texture* texture = &textures[ASSET_TEXTURES_GUI_INDEX];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    polyFT4Render(pol4, 0, ctx);
}