#pragma once

#ifndef PSXMC_BUTTON_H
#define PSXMC_BUTTON_H

#include <stdbool.h>

#include "../ui.h"

typedef enum UIButtonState {
    BUTTON_NONE = 0,
    BUTTON_PRESSED,
    BUTTON_HOVERED,
} UIButtonState;

DEFN_UI_COMPONENT(
    UIButton,
    UIButtonState state;
);

#define UIButton_update_CUSTOM ()
void uiButtonUpdate(VSelf);
void UIButton_update(VSelf);

void uiButtonRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UIButton_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UIButton);
#undef UIButton_update_CUSTOM

#endif // PSXMC_BUTTON_H
