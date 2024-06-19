#pragma once

#ifndef PSX_MINECRAFT_HOTBAR_H
#define PSX_MINECRAFT_HOTBAR_H

#include "../../ui/ui.h"
#include "../../structure/cvector.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"
#include "../../core/input/input.h"
#include "slot.h"

#define HOTBAR_SLOT_COUNT 9
#define HOTBAR_WIDTH 182
#define HOTBAR_HEIGHT 22

#define HOTBAR_SELECTOR_WIDTH 24
#define HOTBAR_SELECTOR_HEIGHT 24

#define hotbarGetSelectSlot(hotbar) ((hotbar)->slots[(hotbar)->selected_slot])

DEFN_UI(
    Hotbar,
    cvector(Slot) slots;
    u8 selected_slot;
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

#endif // PSX_MINECRAFT_HOTBAR_H
