#include "button.h"

#include "../../math/math_utils.h"
#include "../../logging/logging.h"
#include "../../util/memory.h"
#include "cursor.h"

INLINE UIButton* uiButtonNew() {
    UIButton* button = malloc(sizeof(UIButton));
    assert(button != NULL);
    zeroed(button);
    return button;
};

void uiButtonUpdate(VSelf) ALIAS("UIButton_update");
void UIButton_update(VSelf) {
    VSELF(UIButton);
    if (!quadIntersect(
            &cursor.component.position,
            &self->component.position,
            &self->component.dimensions
        )) {
        self->state = BUTTON_NONE;
    }
    switch (cursor.state) {
        case CURSOR_NONE:
            self->state = BUTTON_HOVERED;
            break; 
        case CURSOR_PRESSED:
            self->state = BUTTON_PRESSED;
            break;
        case CURSOR_RELEASED:
            self->state = BUTTON_HOVERED;
            break;
    }
}

void uiButtonRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("UIButton_render");
void UIButton_render(UNUSED VSelf, UNUSED RenderContext* ctx, UNUSED Transforms* transforms) {
    UNIMPLEMENTED();
}
