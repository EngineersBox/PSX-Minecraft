#pragma once

#ifndef PSXMC_CURSOR_H
#define PSXMC_CURSOR_H

#include <interface99.h>

#include "../ui.h"

DEFN_UI_COMPONENT(
    UICursor,
    void* held_data;
);

void uiCursorAction(VSelf, const DVECTOR* cursor_position, bool pressed);
void UICursor_action(VSelf, const DVECTOR* cursor_position, bool pressed);

void uiCursorRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UICursor_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UICursor);

#endif // PSXMC_CURSOR_H
