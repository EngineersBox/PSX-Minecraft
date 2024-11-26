#pragma once

#ifndef PSXMC_INVENTORY_H
#define PSXMC_INVENTORY_H

#include "slot.h"
#include "hotbar.h"
#include "../../structure/cvector.h"
#include "../../ui/ui.h"
#include "../../util/preprocessor.h"
#include "../../util/inttypes.h"
#include "../../hardware/counters.h"

// * 0-3: armour
// * 4-7: crafting input
// * 8: crafting output
// * 9-35: storage
// * [36-44] -> [0-8]: hotbar (via pointer ref)
#define INVENTORY_SLOT_COUNT 45
#define INVENTORY_SLOT_ARMOR_OFFSET 0
#define INVENTORY_SLOT_CRAFTING_OFFSET 4
#define INVENTORY_SLOT_STORAGE_OFFSET 9
#define INVENTORY_SLOT_HOTBAR_OFFSET 36
#define INVENTORY_NO_FREE_SLOT __UINT8_MAX__

#define INVENTORY_WIDTH 176
#define INVENTORY_HEIGHT 166

#define inventorySlotIsRef(slot) ((slot)->index >= INVENTORY_SLOT_HOTBAR_OFFSET)
#define inventorySlotGetItem(slot) ((inventorySlotIsRef(slot) ? (slot)->data.ref->data : (slot)->data).item)
#define inventorySlotSetItem(slot, _item) ({ \
    do { \
        if (inventorySlotIsRef(slot)) { \
            (slot)->data.ref->data.item = (_item); \
        } else { \
            (slot)->data.item = (_item); \
        }\
    } while (0); \
})

#define MK_INVENTORY_STORE_RESULT_LIST(f) \
    f(INVENTORY_STORE_RESULT_ADDED_ALL) \
    f(INVENTORY_STORE_RESULT_ADDED_SOME) \
    f(INVENTORY_STORE_RESULT_NO_SPACE) \
    f(INVENTORY_STORE_RESULT_ADDED_NEW_SLOT)

// Added and freed iitem instance
//  - INVENTORY_STORE_RESULT_ADDED_ALL = 0,
// Added some of stack, didn't free iitem, updated stack_size
//  - INVENTORY_STORE_RESULT_ADDED_SOME,
// No space in inventory, didn't free iitem, didn't update stack size
//  - INVENTORY_STORE_RESULT_NO_SPACE,
// Added to new slot, didn't free iitem
// - INVENTORY_STORE_RESULT_ADDED_NEW_SLOT
typedef enum {
    MK_INVENTORY_STORE_RESULT_LIST(P99_ENUM_ENTRY)
} InventoryStoreResult;

/*extern const char* INVENTORY_STORE_RESULT_NAMES[];*/

/*#define inventoryStoreResultStringify(id) INVENTORY_STORE_RESULT_NAMES[(id)]*/

// Armour slots
#define INVENTORY_ARMOUR_SLOT_GROUP_DIMENSIONS_X 1
#define INVENTORY_ARMOUR_SLOT_GROUP_DIMENSIONS_Y 4
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_SPACING_X 0
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_SPACING_Y 2
#define INVENTORY_ARMOUR_SLOT_GROUP_ORIGIN_X 80
#define INVENTORY_ARMOUR_SLOT_GROUP_ORIGIN_Y 45
slotGroupCheck(INVENTORY_ARMOUR);

// Crafting slots
#define INVENTORY_CRAFTING_SLOT_GROUP_DIMENSIONS_X 2
#define INVENTORY_CRAFTING_SLOT_GROUP_DIMENSIONS_Y 2
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_SPACING_X 2
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_SPACING_Y 2
#define INVENTORY_CRAFTING_SLOT_GROUP_ORIGIN_X 160
#define INVENTORY_CRAFTING_SLOT_GROUP_ORIGIN_Y 63
slotGroupCheck(INVENTORY_CRAFTING);

// Crafting results slots
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_DIMENSIONS_X 1
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_DIMENSIONS_Y 1
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_SPACING_X 0
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_SPACING_Y 0
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_ORIGIN_X 216
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_ORIGIN_Y 73
slotGroupCheck(INVENTORY_CRAFTING_RESULT);

// Main slots
#define INVENTORY_MAIN_SLOT_GROUP_DIMENSIONS_X 9
#define INVENTORY_MAIN_SLOT_GROUP_DIMENSIONS_Y 3
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_SPACING_X 2
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_SPACING_Y 2
#define INVENTORY_MAIN_SLOT_GROUP_ORIGIN_X 80
#define INVENTORY_MAIN_SLOT_GROUP_ORIGIN_Y 121
slotGroupCheck(INVENTORY_MAIN);

// Hotbar slots
#define INVENTORY_HOTBAR_SLOT_GROUP_DIMENSIONS_X 9
#define INVENTORY_HOTBAR_SLOT_GROUP_DIMENSIONS_Y 1
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_SPACING_X 2
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_SPACING_Y 0
#define INVENTORY_HOTBAR_SLOT_GROUP_ORIGIN_X 80
#define INVENTORY_HOTBAR_SLOT_GROUP_ORIGIN_Y 179
slotGroupCheck(INVENTORY_HOTBAR);

#define INVENTORY_DEBOUNCE_MS 200

DEFN_UI(Inventory,
    cvector(Slot) slots;
    Hotbar* hotbar;
    Timestamp debounce;
);

void inventoryInit(Inventory* inventory, Hotbar* hotbar);

void inventoryRenderSlots(const Inventory* inventory, RenderContext* ctx, Transforms* transforms);

Slot* inventorySearchItem(const Inventory* inventory, const ItemID id, const u8 from_slot, u8* next_free);
Slot* inventoryFindFreeSlot(const Inventory* inventory, const u8 from_slot);

InventoryStoreResult inventoryStoreItem(const Inventory* inventory, IItem* iitem);

void inventoryOpen(VSelf);
void Inventory_open(VSelf);

void inventoryClose(VSelf);
void Inventory_close(VSelf);

void inventoryRegisterInputHandler(VSelf, Input* input, void* ctx);
void Inventory_registerInputHandler(VSelf, Input* input, void* ctx);

impl(IInputHandler, Inventory);
impl(IUI, Inventory);

#endif // PSXMC_INVENTORY_H
