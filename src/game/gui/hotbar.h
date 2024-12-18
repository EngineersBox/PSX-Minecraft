#pragma once

#ifndef PSXMC_HOTBAR_H
#define PSXMC_HOTBAR_H

#include "../../ui/ui.h"
#include "../../structure/cvector.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "../../core/input/input.h"
#include "../../hardware/counters.h"
#include "slot.h"

#define HOTBAR_SLOT_GROUP_DIMENSIONS_X 9
#define HOTBAR_SLOT_GROUP_DIMENSIONS_Y 1
#define HOTBAR_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define HOTBAR_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define HOTBAR_SLOT_GROUP_SLOT_SPACING_X 4
#define HOTBAR_SLOT_GROUP_SLOT_SPACING_Y 0
#define HOTBAR_SLOT_GROUP_ORIGIN_X 72
#define HOTBAR_SLOT_GROUP_ORIGIN_Y 220
#define HOTBAR_SLOT_GROUP_INDEX_OFFSET 0
slotGroupCheck(HOTBAR);

#define HOTBAR_SLOT_COUNT slotGroupSize(HOTBAR)
#define HOTBAR_WIDTH 182
#define HOTBAR_HEIGHT 22

#define HOTBAR_SELECTOR_WIDTH 24
#define HOTBAR_SELECTOR_HEIGHT 24

#define HOTBAR_DEBOUNCE_MS 200

#define hotbarGetSelectSlot(hotbar) ((hotbar)->slots[(hotbar)->selected_slot])

DEFN_UI(
    Hotbar,
    Slot slots[HOTBAR_SLOT_COUNT];
    u8 selected_slot;
    Timestamp debounce;
);

void hotbarInit(Hotbar* hotbar);

void hotbarRenderSlots(const Hotbar* hotbar, RenderContext* ctx, Transforms* transforms);

void hotbarOpen(VSelf);
void Hotbar_open(VSelf);

void hotbarClose(VSelf);
void Hotbar_close(VSelf);

void hotbarRegisterInputHandler(VSelf, Input* input, void* ctx);
void Hotbar_registerInputHandler(VSelf, Input* input, void* ctx);

impl(IInputHandler, Hotbar);
impl(IUI, Hotbar);

#endif // PSXMC_HOTBAR_H
