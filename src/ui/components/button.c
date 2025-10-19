#include "button.h"

#include <psxgpu.h>

#include "../../logging/logging.h"
#include "../../math/math_utils.h"
#include "../../render/font.h"
#include "../../resources/asset_indices.h"
#include "../../resources/assets.h"
#include "../../resources/texture.h"
#include "../../structure/primitive/primitive.h"
#include "cursor.h"

UIButton* uiButtonNew(const char* text,
                      i16 x,
                      i16 y,
                      i16 width,
                      u8 ot_entry_index) {
    UIButton* button = (UIButton*) malloc(sizeof(UIButton));
    assert(button != NULL);
    button->component.position = vec2_i16(x, y);
    button->component.dimensions = vec2_i16(width, UI_BUTTON_HEIGHT);
    button->text = text;
    button->state = BUTTON_NONE;
    button->ot_entry_index = ot_entry_index;
    return button;
};

void uiButtonUpdate(VSelf) ALIAS("UIButton_update");
void UIButton_update(VSelf) {
    VSELF(UIButton);
    if (self->state == BUTTON_DISABLED) {
        return;
    }
    if (!quadIntersect(
            &cursor.component.position,
            &self->component.position,
            &self->component.dimensions
        )) {
        self->state = BUTTON_NONE;
    }
    switch (cursor.state) {
        case CURSOR_NONE:
            FALLTHROUGH;
        case CURSOR_RELEASED:
            self->state = BUTTON_HOVERED;
            break;
        case CURSOR_PRESSED:
            self->state = BUTTON_PRESSED;
            break;
    }
}

void uiButtonRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("UIButton_render");
void UIButton_render(VSelf, RenderContext* ctx, UNUSED Transforms* transforms) {
    VSELF(UIButton);
    DEBUG_LOG("[BUTTON] Start render\n");
    DEBUG_LOG("[BUTTON] Print text\n");
    fontPrintCentreOffset(
        ctx,
        self->component.position.vx + (self->component.dimensions.vx >> 1),
        self->component.position.vy + (self->component.dimensions.vy >> 1) - (FONT_CHARACTER_SPRITE_HEIGHT >> 1),
        0,
        0,
        self->text
    );
    DEBUG_LOG("[BUTTON] Allocate primitive\n");
    POLY_FT4* poly_ft4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        poly_ft4,
        self->component.position.vx,
        self->component.position.vy,
        self->component.dimensions.vx,
        self->component.dimensions.vy
    );
    switch (self->state) {
        case BUTTON_NONE:
            setUVWH(
                poly_ft4,
                UI_BUTTON_NONE_U,
                UI_BUTTON_NONE_V,
                UI_BUTTON_TEXTURE_WIDTH,
                UI_BUTTON_TEXTURE_HEIGHT
            );
            break;
        case BUTTON_PRESSED:
            setUVWH(
                poly_ft4,
                UI_BUTTON_PRESSED_U,
                UI_BUTTON_PRESSED_V,
                UI_BUTTON_TEXTURE_WIDTH,
                UI_BUTTON_TEXTURE_HEIGHT
            );
            break;
        case BUTTON_HOVERED:
            setUVWH(
                poly_ft4,
                UI_BUTTON_HOVERED_U,
                UI_BUTTON_HOVERED_V,
                UI_BUTTON_TEXTURE_WIDTH,
                UI_BUTTON_TEXTURE_HEIGHT
            );
            break;
        case BUTTON_DISABLED:
            setUVWH(
                poly_ft4,
                UI_BUTTON_DISABLED_U,
                UI_BUTTON_DISABLED_V,
                UI_BUTTON_TEXTURE_WIDTH,
                UI_BUTTON_TEXTURE_HEIGHT
            );
            break;
    }
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI_PART];
    poly_ft4->tpage = texture->tpage;
    poly_ft4->clut = texture->clut;
    polyFT4Render(
        poly_ft4,
        self->ot_entry_index,
        ctx
    );
    DEBUG_LOG("[BUTTON] End render\n");
}
