#pragma once

#ifndef PSX_MINECRAFT_INVENTORY_H
#define PSX_MINECRAFT_INVENTORY_H

#include "slot.h"
#include "hotbar.h"
#include "../../structure/cvector.h"
#include "../../ui/ui.h"

// * 0-3: armor
// * 4-7: crafting input
// * 8: crafting output
// * 9-35: storage
// * [36-44] -> [0-8]: hotbar (via pointer, not included in count)
#define INVENTORY_SLOT_COUNT 36

DEFN_UI(Inventory,
    cvector(Slot) slots;
    Hotbar* hotbar;
);

void inventoryRenderSlots(const Inventory* inventory);

void inventoryLoadTexture(VSelf);
void Inventory_loadTexture(VSelf);

void inventoryFreeTexture(VSelf);
void Inventory_freeTexture(VSelf);

void inventoryInit(Inventory* inventory, Hotbar* hotbar);

impl(IUI, Inventory);

#endif // PSX_MINECRAFT_INVENTORY_H
