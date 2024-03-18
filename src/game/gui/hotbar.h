#pragma once

#ifndef PSX_MINECRAFT_HOTBAR_H
#define PSX_MINECRAFT_HOTBAR_H

#include "../../ui/ui.h"
#include "../../structure/cvector.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "slot.h"

#define HOTBAR_SLOT_COUNT 9
#define HOTBAR_WIDTH 180
#define HOTBAR_HEIGHT 19

DEFN_UI(
    Hotbar,
    cvector(Slot) slots;
);

void hotbarInit(Hotbar* hotbar);

void hotbarLoadTexture(VSelf);
void Hotbar_loadTexture(VSelf);

void hotbarFreeTexture(VSelf);
void Hotbar_freeTexture(VSelf);

impl(IUI, Hotbar);

#endif // PSX_MINECRAFT_HOTBAR_H
