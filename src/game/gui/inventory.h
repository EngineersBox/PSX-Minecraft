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

typedef struct {
    UI ui;
    cvector(Slot) slots;
    Hotbar* hotbar;
} Inventory;

void inventoryInit(Inventory* inventory, Hotbar* hotbar);
void inventoryRender(const Inventory* inventory, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_INVENTORY_H
