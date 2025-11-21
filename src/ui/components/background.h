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
    const Texture* texture;
    DVECTOR texture_coords;
    DVECTOR texture_dimensions;
    CVECTOR tint;
    u8 ot_entry_index;
);

UIBackground* uiBackgroundNew(const Texture* texture,
                              const DVECTOR position,
                              const DVECTOR dimensions,
                              const DVECTOR texture_coords,
                              const DVECTOR texture_dimensions,
                              const CVECTOR tint,
                              u8 ot_entry_index);

void uiBackgroundRender(VSelf, RenderContext* ctx, Transforms* transforms);
void UIBackground_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IUIComponent, UIBackground);

#endif // PSXMC__UI_COMPONENTS__BACKGROUND_H
