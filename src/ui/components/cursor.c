#include "cursor.h"

#include <psxgpu.h>
#include <psxgte.h>

#include "../../core/input/input.h"
#include "../../math/math_utils.h"
#include "../../resources/asset_indices.h"
#include "../../resources/assets.h"

void uiCursorUpdate(VSelf) ALIAS("UICursor_update");
void UICursor_update(VSelf) {
    VSELF(UICursor);
    i16 delta_x = 0;
    if (isPressed(input.pad, BINDING_MOVE_FORWARD)) {
        delta_x = -CURSOR_MOVE_PIXELS_DELTA;
    } else if (isPressed(input.pad, BINDING_MOVE_BACKWARD)) {
        delta_x = CURSOR_MOVE_PIXELS_DELTA;
    }
    self->component.position.vx = min(0, max(
        SCREEN_XRES,
        self->component.position.vx + delta_x
    ));
    i16 delta_y = 0;
    if (isPressed(input.pad, BINDING_MOVE_LEFT)) {
        delta_y = -CURSOR_MOVE_PIXELS_DELTA;
    } else if (isPressed(input.pad, BINDING_MOVE_RIGHT)) {
        delta_y = CURSOR_MOVE_PIXELS_DELTA;
    }
    self->component.position.vy = min(0, max(
        SCREEN_YRES,
        self->component.position.vy + delta_y
    ));
}

void uiCursorRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("UICursor_render");
void UICursor_render(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(UICursor);
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setXYWH(
        pol4,
        self->component.position.vx,
        self->component.position.vy,
        CURSOR_SPRITE_WIDTH,
        CURSOR_SPRITE_HEIGHT
    );
    setUVWH(
        pol4,
        CURSOR_SPRITE_POS_X,
        CURSOR_SPRITE_POS_Y,
        CURSOR_SPRITE_WIDTH,
        CURSOR_SPRITE_HEIGHT
    );
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    const u32* ot_object = allocateOrderingTable(ctx, 0);
    addPrim(ot_object, pol4);
}
