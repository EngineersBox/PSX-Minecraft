#pragma once

#ifndef PSX_MINECRAFT_INVENTORY_H
#define PSX_MINECRAFT_INVENTORY_H

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

#define MK_INVENTORY_STORE_RESULT_LSIT(f) \
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
    MK_INVENTORY_STORE_RESULT_LSIT(P99_ENUM_ENTRY)
} InventoryStoreResult;

extern const char* INVENTORY_STORE_RESULT_NAMES[];

#define inventoryStoreResultStringify(id) INVENTORY_STORE_RESULT_NAMES[(id)]

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

#endif // PSX_MINECRAFT_INVENTORY_H
