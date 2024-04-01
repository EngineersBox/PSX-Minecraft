#include "cursor.h"

void uiCursorAction(VSelf, const DVECTOR* cursor_position, bool pressed) __attribute__((alias("UICursor_action")));
void UICursor_action(VSelf, const DVECTOR* cursor_position, bool pressed) {

}

void uiCursorRender(VSelf, RenderContext* ctx, Transforms* transforms) __attribute__((alias("UICursor_render")));
void UICursor_render(VSelf, RenderContext* ctx, Transforms* transforms) {

}