#pragma once

#ifndef PSX_MINECRAFT_INVENTORY_H
#define PSX_MINECRAFT_INVENTORY_H

#include "slot.h"
#include "hotbar.h"
#include "../../structure/cvector.h"
#include "../../ui/ui.h"
#include "../../util/preprocessor.h"

// * 0-3: armor
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
#define inventorySlotGetItem(slot) (inventorySlotIsRef(slot) ? (slot)->data.ref->data.item : (slot)->data.item)

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

DEFN_UI(Inventory,
    cvector(Slot) slots;
    Hotbar* hotbar;
);

void inventoryInit(Inventory* inventory, Hotbar* hotbar);

void inventoryRenderSlots(const Inventory* inventory, RenderContext* ctx, Transforms* transforms);

Slot* inventorySearchItem(const Inventory* inventory, const ItemID id, const uint8_t from_slot, uint8_t* next_free);
Slot* inventoryFindFreeSlot(const Inventory* inventory, const uint8_t from_slot);

InventoryStoreResult inventoryStoreItem(Inventory* inventory, IItem* iitem);

void inventoryLoadTexture(VSelf);
void Inventory_loadTexture(VSelf);

void inventoryFreeTexture(VSelf);
void Inventory_freeTexture(VSelf);

impl(IUI, Inventory);

#endif // PSX_MINECRAFT_INVENTORY_H
