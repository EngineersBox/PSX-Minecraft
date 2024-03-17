#pragma once

#ifndef PSX_MINECRAFT_INVENTORY_H
#define PSX_MINECRAFT_INVENTORY_H

#include <stdint.h>
#include <stdbool.h>
#include <psxgte.h>

#include "../../structure/svector.h"
#include "../items/item.h"
#include "../../ui/ui.h"

typedef struct {
    IItem* item;
    DVECTOR position;
    DVECTOR dimensions;
    bool blocked;
} Slot;

typedef struct {
    UI ui;
    cvector(Slot) slots;
} Inventory;

#endif // PSX_MINECRAFT_INVENTORY_H
