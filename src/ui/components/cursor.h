#pragma once

#ifndef PSX_MINECRAFT_CURSOR_H
#define PSX_MINECRAFT_CURSOR_H

#include <interface99.h>

#include "../ui.h"

DEFN_UI_COMPONENT(
    UICursor,
    void* held_data
);

void uiCursorAction(VSelf, const DVECTOR* cursor_position, const bool pressed);
void UICursor_action(VSelf, const DVECTOR* cursor_position, const bool pressed);

void uiCursorRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UICursor_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UICursor);

#endif // PSX_MINECRAFT_CURSOR_H
