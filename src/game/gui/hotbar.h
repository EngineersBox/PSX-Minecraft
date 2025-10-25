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

#define HOTBAR_ICON_COUNT 10

// NOTE: Icon heights are 10, since SPRT primitives
//       need to have dimensions of a multiple of 2

#define HOTBAR_ATTRIBUTE_Y_OFFSET 3

#define HOTBAR_HEALTH_ICON_COUNT 10
#define HOTBAR_HEALTH_ICON_WIDTH 9
#define HOTBAR_HEALTH_ICON_HALF_WIDTH 5
#define HOTBAR_HEALTH_ICON_HEIGHT 10
#define HOTBAR_HEALTH_ORIGIN_POS_X (CENTRE_X - (HOTBAR_WIDTH >> 1))
#define HOTBAR_HEALTH_ORIGIN_POS_Y (SCREEN_YRES - (HOTBAR_HEIGHT) - HOTBAR_HEALTH_ICON_HEIGHT - HOTBAR_ATTRIBUTE_Y_OFFSET)

#define HOTBAR_HEALTH_BLACK_U 0
#define HOTBAR_HEALTH_BLACK_V 82

#define HOTBAR_HEALTH_WHITE_U 90
#define HOTBAR_HEALTH_WHITE_V 82

#define HOTBAR_HEALTH_FILL_U 0
#define HOTBAR_HEALTH_FILL_V 92

#define HOTBAR_HEALTH_FILL_LIGHT_U 90
#define HOTBAR_HEALTH_FILL_LIGHT_V 92

#define HOTBAR_ARMOUR_ICON_COUNT 10
#define HOTBAR_ARMOUR_ICON_WIDTH 9
#define HOTBAR_ARMOUR_ICON_HALF_WIDTH 5
#define HOTBAR_ARMOUR_ICON_HEIGHT 10
#define HOTBAR_ARMOUR_ORIGIN_POS_X (CENTRE_X + (HOTBAR_WIDTH >> 1) - (HOTBAR_ICON_COUNT * HOTBAR_ARMOUR_ICON_WIDTH))
#define HOTBAR_ARMOUR_ORIGIN_POS_Y (SCREEN_YRES - (HOTBAR_HEIGHT) - HOTBAR_ARMOUR_ICON_HEIGHT - HOTBAR_ATTRIBUTE_Y_OFFSET)
#define HOTBAR_ARMOUR_U 0
#define HOTBAR_ARMOUR_V 102
#define HOTBAR_ARMOUR_FILL_U 90
#define HOTBAR_ARMOUR_FILL_V 102

#define HOTBAR_AIR_ICON_COUNT 10
#define HOTBAR_AIR_ICON_WIDTH 9
#define HOTBAR_AIR_ICON_HEIGHT 10
#define HOTBAR_AIR_ORIGIN_POS_X (CENTRE_X - (HOTBAR_WIDTH >> 1))
#define HOTBAR_AIR_ORIGIN_POS_Y (HOTBAR_HEALTH_ORIGIN_POS_Y - HOTBAR_AIR_ICON_HEIGHT)
#define HOTBAR_AIR_U 0
#define HOTBAR_AIR_V 112
#define HOTBAR_AIR_POP_U 90
#define HOTBAR_AIR_POP_V 112

#define hotbarGetSelectSlot(hotbar) ((hotbar)->slots[(hotbar)->selected_slot])

DEFN_UI(
    Hotbar,
    Slot slots[HOTBAR_SLOT_COUNT];
    u8 selected_slot;
    Timestamp debounce;
);

Hotbar* hotbarNew();
void hotbarInit(Hotbar* hotbar);

void hotbarRenderSlots(const Hotbar* hotbar, RenderContext* ctx, Transforms* transforms);
void hotbarRenderAttributes(u8 health,
                            bool health_start_flash,
                            u8 armour,
                            u8 air,
                            bool in_water,
                            RenderContext* ctx,
                            Transforms* transforms);

void hotbarRegisterInputHandler(VSelf, Input* input, void* ctx);
void Hotbar_registerInputHandler(VSelf, Input* input, void* ctx);

impl(IInputHandler, Hotbar);
impl(IUI, Hotbar);

#endif // PSXMC_HOTBAR_H
