#include "button.h"

#include "../../math/math_utils.h"

void uiButtonAction(VSelf, const DVECTOR* cursor_position, const bool pressed) __attribute__((alias("UIButton_action")));
void UIButton_action(VSelf, const DVECTOR* cursor_position, const bool pressed) {
    VSELF(UIButton);
    if (!quadIntersect(cursor_position, &self->component.position, &self->component.dimensions)) {
        self->pressed = false;
        return;
    }
    self->pressed = pressed;
}

void uiButtonRender(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("UIButton_render")));
void UIButton_render(VSelf, RenderContext* ctx, Transforms* transforms) {

}
