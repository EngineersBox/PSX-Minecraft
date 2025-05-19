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
#include "utils.h"

#define INVENTORY_NO_FREE_SLOT __UINT8_MAX__

#define INVENTORY_WIDTH 176
#define INVENTORY_HEIGHT 166

#define inventorySlotIsRef(slot) ((slot)->index >= slotGroupIndexOffset(INVENTORY_HOTBAR))
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
    f(INVENTORY_STORE_RESULT_ADDED_NEW_SLOT) \
    f(INVENTORY_STORE_RESULT_NO_SPACE)

// Added and freed iitem instance
//  - INVENTORY_STORE_RESULT_ADDED_ALL = 0
// Added some of stack, didn't free iitem, updated stack_size
//  - INVENTORY_STORE_RESULT_ADDED_SOME
// Added to new slot, didn't free iitem
// - INVENTORY_STORE_RESULT_ADDED_NEW_SLOT
// No space in inventory, didn't free iitem, didn't update stack size
//  - INVENTORY_STORE_RESULT_NO_SPACE
typedef enum InventoryStoreResult {
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
#define INVENTORY_ARMOUR_SLOT_GROUP_INDEX_OFFSET 0
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
#define INVENTORY_CRAFTING_SLOT_GROUP_INDEX_OFFSET 4
slotGroupCheck(INVENTORY_CRAFTING);

// Crafting result slots
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_DIMENSIONS_X 1
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_DIMENSIONS_Y 1
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_SPACING_X 0
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_SLOT_SPACING_Y 0
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_ORIGIN_X 216
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_ORIGIN_Y 73
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_INDEX_OFFSET 8
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
#define INVENTORY_MAIN_SLOT_GROUP_INDEX_OFFSET 9
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
#define INVENTORY_HOTBAR_SLOT_GROUP_INDEX_OFFSET 36
slotGroupCheck(INVENTORY_HOTBAR);

#define INVENTORY_DEBOUNCE_MS 50

// * 0-3: armour
// * 4-7: crafting input
// * 8: crafting output
// * 9-35: storage
// * [36-44] -> [0-8]: hotbar (via pointer ref)
#define INVENTORY_SLOT_COUNT (\
    slotGroupSize(INVENTORY_ARMOUR) \
    + slotGroupSize(INVENTORY_CRAFTING) \
    + slotGroupSize(INVENTORY_CRAFTING_RESULT) \
    + slotGroupSize(INVENTORY_MAIN) \
    + slotGroupSize(INVENTORY_HOTBAR) \
)

typedef enum InventorySlotGroup {
    INVENTORY_SLOT_GROUP_ARMOUR = 0b00001,
    INVENTORY_SLOT_GROUP_CRAFTING = 0b00010,
    INVENTORY_SLOT_GROUP_CRAFTING_RESULT = 0b00100,
    INVENTORY_SLOT_GROUP_MAIN = 0b01000,
    INVENTORY_SLOT_GROUP_HOTBAR = 0b10000,
    INVENTORY_SLOT_GROUP_NONE = 0b00000,
    INVENTORY_SLOT_GROUP_ALL = 0b11111
} InventorySlotGroup;

// Elements of InventorySlotGroup bundled together
// via logical OR (|)
typedef u8 InventorySlotGroups;

DEFN_UI(Inventory,
    Slot slots[INVENTORY_SLOT_COUNT];
    Hotbar* hotbar;
    Timestamp debounce;
);

Inventory* inventoryNew();
void inventoryInit(Inventory* inventory, Hotbar* hotbar);

// Groups indicates which slot groups to render,
// which should be a logical OR of all the desired
// groups in the InventorySlotGroup enum
void inventoryRenderSlots(const Inventory* inventory,
                          InventorySlotGroups groups,
                          RenderContext* ctx,
                          Transforms* transforms);

Slot* inventorySearchItem(Inventory* inventory,
                          const ItemID id,
                          const u8 metadata_id,
                          const u8 from_slot,
                          u8* next_free);
Slot* inventoryFindFreeSlot(Inventory* inventory, const u8 from_slot);

InventoryStoreResult inventoryStoreItem(Inventory* inventory, IItem* iitem);

void inventoryOpen(VSelf);
void Inventory_open(VSelf);

void inventoryClose(VSelf);
void Inventory_close(VSelf);

void inventoryRegisterInputHandler(VSelf, Input* input, void* ctx);
void Inventory_registerInputHandler(VSelf, Input* input, void* ctx);

void inventoryCursorHandler(Inventory* inventory,
                            InventorySlotGroups groups,
                            const Input* input);


IItem* inventorySlotItemGetter(Slot* slot);
void inventorySlotItemSetter(Slot* slot, IItem* item);

impl(IInputHandler, Inventory);
impl(IUI, Inventory);

#endif // PSXMC_INVENTORY_H
