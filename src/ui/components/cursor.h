#pragma once

#ifndef PSXMC_CURSOR_H
#define PSXMC_CURSOR_H

#include <interface99.h>

#include "../ui.h"

#define CURSOR_MOVE_PIXELS_DELTA 4
#define CURSOR_SPRITE_WIDTH 16
#define CURSOR_SPRITE_HEIGHT 16
#define CURSOR_SPRITE_POS_X 182
#define CURSOR_SPRITE_POS_Y 24

typedef enum CursorState {
    CURSOR_NONE = 0,
    CURSOR_PRESSED,
    CURSOR_RELEASED
} CursorState;

DEFN_UI_COMPONENT(
    UICursor,
    CursorState state;
    void* held_data;
);

#define UICursor_update_CUSTOM ()
void uiCursorUpdate(VSelf);
void UICursor_update(VSelf);

void uiCursorRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UICursor_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UICursor);
#undef UICursor_update_CUSTOM

extern UICursor cursor;
extern IUIComponent cursor_component;

#endif // PSXMC_CURSOR_H
