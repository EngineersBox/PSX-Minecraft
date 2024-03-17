#pragma once

#ifndef PSX_MINECRAFT_BUTTON_H
#define PSX_MINECRAFT_BUTTON_H

#include <stdbool.h>

#include "../ui.h"

DEFN_UI_COMPONENT(
    Button,
    bool pressed;
);

void buttonAction(VSelf, const DVECTOR* cursor_position, const bool pressed);
void Button_action(VSelf, const DVECTOR* cursor_position, const bool pressed);

void buttonRender(VSelf, RenderContext* ctx, Transforms* transforms);
void Button_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, Button);

#endif // PSX_MINECRAFT_BUTTON_H
