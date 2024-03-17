#pragma once

#ifndef PSX_MINECRAFT_HOTBAR_H
#define PSX_MINECRAFT_HOTBAR_H

#include "../../ui/ui.h"
#include "../../structure/cvector.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "slot.h"

#define HOTBAR_SLOT_COUNT 9

typedef struct {
    UI ui;
    cvector(Slot) slots;
} Hotbar;

void hotbarInit(Hotbar* hotbar);
void hotbarRender(const Hotbar* hotbar, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_HOTBAR_H
