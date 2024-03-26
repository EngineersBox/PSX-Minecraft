#pragma once

#ifndef PSX_MINECRAFT_BACKGROUND_H
#define PSX_MINECRAFT_BACKGROUND_H

#include <stdint.h>
#include <psxgte.h>

#include "../ui.h"
#include "../../resources/asset_indices.h"
#include "../../resources/assets.h"

DEFN_UI_COMPONENT(
    UIBackground,
    Texture* texture;
    DVECTOR texture_coords;
    DVECTOR texture_width;
);

void uiBackgroundAction(VSelf, const DVECTOR* cursor_position, const bool pressed);
void UIBackground_action(VSelf, const DVECTOR* cursor_position, const bool pressed);

void uiBackgroundRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UIBackground_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UIBackground);

#endif // PSX_MINECRAFT_BACKGROUND_H
