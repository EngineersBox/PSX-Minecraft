#include "button.h"

#include "../../math/math_utils.h"

void buttonAction(VSelf, const DVECTOR* cursor_position, const bool pressed) __attribute__((alias("Button_action")));
void Button_action(VSelf, const DVECTOR* cursor_position, const bool pressed) {
    VSELF(Button);
    if (!quadIntersect(cursor_position, &self->component.position, &self->component.dimensions)) {
        self->pressed = false;
        return;
    }
    self->pressed = pressed;
}

void buttonRender(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("Button_render")));
void Button_render(VSelf, RenderContext* ctx, Transforms* transforms) {

}
