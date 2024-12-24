#pragma once

#ifndef PSXMC__UI_COMPONENTS__BACKGROUND_H
#define PSXMC__UI_COMPONENTS__BACKGROUND_H

#include <stdint.h>
#include <psxgte.h>

#include "../ui.h"
#include "../../resources/asset_indices.h"
#include "../../resources/assets.h"

DEFN_UI_COMPONENT(
    UIBackground,
    Texture texture;
    DVECTOR texture_coords;
    DVECTOR texture_width;
);

void uiBackgroundRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UIBackground_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UIBackground);

#endif // PSXMC__UI_COMPONENTS__BACKGROUND_H
