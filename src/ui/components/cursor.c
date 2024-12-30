#include "cursor.h"

#include <psxgpu.h>
#include <psxgte.h>

#include "../../core/input/input.h"
#include "../../math/math_utils.h"
#include "../../resources/asset_indices.h"
#include "../../resources/assets.h"
#include "../../game/items/item.h"
#include "../../util/interface99_extensions.h"

UICursor cursor = {
    .component = {
        .position = vec2_i16(
            CENTRE_X,
            CENTRE_Y
        ),
        .dimensions = vec2_i16(
            CURSOR_SPRITE_WIDTH,
            CURSOR_SPRITE_HEIGHT
        )
    },
    .held_data = NULL,
    .state = CURSOR_NONE
};
IUIComponent cursor_component = DYN(UICursor, IUIComponent, &cursor);

void uiCursorUpdate(VSelf) ALIAS("UICursor_update");
void UICursor_update(VSelf) {
    VSELF(UICursor);
    i16 delta_x = 0;
    if (isPressed(input.pad, BINDING_MOVE_LEFT)) {
        delta_x = -CURSOR_MOVE_PIXELS_DELTA;
    } else if (isPressed(input.pad, BINDING_MOVE_RIGHT)) {
        delta_x = CURSOR_MOVE_PIXELS_DELTA;
    }
    self->component.position.vx = max(0, min(
        SCREEN_XRES,
        self->component.position.vx + delta_x
    ));
    i16 delta_y = 0;
    if (isPressed(input.pad, BINDING_MOVE_FORWARD)) {
        delta_y = -CURSOR_MOVE_PIXELS_DELTA;
    } else if (isPressed(input.pad, BINDING_MOVE_BACKWARD)) {
        delta_y = CURSOR_MOVE_PIXELS_DELTA;
    }
    self->component.position.vy = max(0, min(
        SCREEN_YRES,
        self->component.position.vy + delta_y
    ));
    if (self->held_data == NULL) {
        return;
    }
    Item* item = VCAST_PTR(Item*, (IItem*) self->held_data);
    item->position.vx = self->component.position.vx;
    item->position.vy = self->component.position.vy;
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
        self->component.dimensions.vx,
        self->component.dimensions.vy
    );
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    const u32* ot_object = allocateOrderingTable(ctx, 0);
    addPrim(ot_object, pol4);
    if (self->held_data == NULL) {
        return;
    }
    IItem* item = (IItem*) self->held_data;
    VCALL_SUPER(*item, Renderable, renderInventory, ctx, transforms);
}

void uiCursorSetHeldData(VSelf, void* data) {
    VSELF(UICursor);
    self->held_data = data;
    if (data == NULL) {
        return;
    }
    IItem* iitem = (IItem*) data;
    Item* item = VCAST_PTR(Item*, iitem);
    VCALL_SUPER(*iitem, Renderable, applyInventoryRenderAttributes);
    item->position.vx = self->component.position.vx;
    item->position.vy = self->component.position.vy;
}
