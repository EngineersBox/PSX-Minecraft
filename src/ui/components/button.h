#pragma once

#ifndef PSX_MINECRAFT_BUTTON_H
#define PSX_MINECRAFT_BUTTON_H

#include <stdbool.h>

#include "../ui.h"

DEFN_UI_COMPONENT(
    UIButton,
    bool pressed;
);

void uiButtonAction(VSelf, const DVECTOR* cursor_position, const bool pressed);
void UIButton_action(VSelf, const DVECTOR* cursor_position, const bool pressed);

void uiButtonRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UIButton_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UIButton);

#endif // PSX_MINECRAFT_BUTTON_H
