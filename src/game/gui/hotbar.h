#pragma once

#ifndef PSX_MINECRAFT_HOTBAR_H
#define PSX_MINECRAFT_HOTBAR_H

#include "../../ui/ui.h"
#include "../../structure/cvector.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "slot.h"

#define HOTBAR_SLOT_COUNT 9
#define HOTBAR_WIDTH 182
#define HOTBAR_HEIGHT 22

#define HOTBAR_SELECTOR_WIDTH 24
#define HOTBAR_SELECTOR_HEIGHT 24

DEFN_UI(
    Hotbar,
    cvector(Slot) slots;
    uint8_t selected_slot;
);

void hotbarInit(Hotbar* hotbar);

void hotbarRenderSlots(const Hotbar* hotbar, RenderContext* ctx, Transforms* transforms);

void hotbarOpen(VSelf);
void Hotbar_open(VSelf);

void hotbarClose(VSelf);
void Hotbar_close(VSelf);

impl(IUI, Hotbar);

#endif // PSX_MINECRAFT_HOTBAR_H
