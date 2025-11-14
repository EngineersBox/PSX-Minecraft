#pragma once

#ifndef PSXMC_BUTTON_H
#define PSXMC_BUTTON_H

#include <stdbool.h>

#include "../ui.h"

#define UI_BUTTON_TEXTURE_WIDTH 200
#define UI_BUTTON_TEXTURE_HEIGHT 20

#define UI_BUTTON_HEIGHT UI_BUTTON_TEXTURE_HEIGHT

#define UI_BUTTON_NONE_U 0
#define UI_BUTTON_NONE_V 42

#define UI_BUTTON_PRESSED_U 0
#define UI_BUTTON_PRESSED_V 62

#define UI_BUTTON_HOVERED_U UI_BUTTON_PRESSED_U
#define UI_BUTTON_HOVERED_V UI_BUTTON_PRESSED_V

#define UI_BUTTON_DISABLED_U 0
#define UI_BUTTON_DISABLED_V 22

typedef enum UIButtonState {
    BUTTON_NONE = 0,
    BUTTON_PRESSED,
    BUTTON_HOVERED,
    BUTTON_DISABLED
} UIButtonState;

DEFN_UI_COMPONENT(
    UIButton,
    const char* text;
    UIButtonState state: 8;
    u8 ot_entry_index: 8;
);

UIButton* uiButtonNew(const char* text,
                      const DVECTOR position,
                      i16 width,
                      u8 ot_entry_index);

#define UIButton_update_CUSTOM ()
void uiButtonUpdate(VSelf);
void UIButton_update(VSelf);

void uiButtonRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UIButton_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UIButton);

#endif // PSXMC_BUTTON_H
