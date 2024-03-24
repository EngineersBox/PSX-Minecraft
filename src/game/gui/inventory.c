#include "inventory.h"

#include <interface99.h>
#include "../../util/interface99_extensions.h"

const char* INVENTORY_STORE_RESULT_NAMES[] = {
    MK_INVENTORY_STORE_RESULT_LSIT(P99_STRING_ARRAY_INDEX)
};

void inventoryRenderSlots(const Inventory* inventory) {
    if (!inventory->ui.active) {
        return;
    }
}

Slot* inventorySearchItem(const Inventory* inventory, const ItemID id, const uint8_t from_slot, uint8_t* next_free) {
    *next_free = INVENTORY_NO_FREE_SLOT;
    if (from_slot >= INVENTORY_SLOT_COUNT) {
        return NULL;
    }
    for (uint8_t i = max(from_slot, INVENTORY_SLOT_HOTBAR_OFFSET); i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        Slot* slot_ref = slot->data.ref;
        if (slot_ref->data.item == NULL) {
            if (*next_free == INVENTORY_NO_FREE_SLOT) {
                *next_free = i;
            }
            continue;
        }
        const Item* item = VCAST(Item*, *slot_ref->data.item);
        if (item->id == id) {
            return slot;
        }
    }
    for (uint8_t i = from_slot; i < INVENTORY_SLOT_HOTBAR_OFFSET; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->data.item == NULL) {
            if (*next_free == INVENTORY_NO_FREE_SLOT) {
                *next_free = i;
            }
            continue;
        }
        const Item* item = VCAST(Item*, *slot->data.item);
        if (item->id == id) {
            return slot;
        }
    }
    return NULL;
}

Slot* inventoryFindFreeSlot(const Inventory* inventory, const uint8_t from_slot) {
    if (from_slot >= INVENTORY_SLOT_COUNT) {
        return NULL;
    }
    for (uint8_t i = max(from_slot, INVENTORY_SLOT_HOTBAR_OFFSET); i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->data.ref->data.item == NULL) {
            return slot;
        }
    }
    for (uint8_t i = from_slot; i < INVENTORY_SLOT_HOTBAR_OFFSET; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->data.item == NULL) {
            return slot;
        }
    }
    return NULL;
}

InventoryStoreResult inventoryStoreItem(Inventory* inventory, IItem* iitem) {
    // 1. Does the item already exist in the inventory?
    //   a. [1:TRUE] Does the existing have space?
    //     i. [a:TRUE] Add the item quantity to the existing stack (up to max)
    //     ii. [a:TRUE] Is there some items left over?
    //       1_1. [ii:TRUE] Duplicate this item instance and go to 1
    //       1_2: [ii:FALSE] Done
    //     iii. [a:FALSE] Go to 2
    //   b. [1:FALSE] Go to 2
    // 2. Is there space in the inventory
    //   a. [2:TRUE] Add the item stack into the next free slot
    //   b. [2:FALSE] Add item back into array at the same index (make sure item world position is correct)
    InventoryStoreResult exit_code = INVENTORY_STORE_RESULT_ADDED_ALL;
    IItem* iitem_to_add = iitem;
    uint8_t from_slot = INVENTORY_SLOT_STORAGE_OFFSET;
    uint8_t next_free = INVENTORY_NO_FREE_SLOT;
    uint8_t persisted_next_free = INVENTORY_NO_FREE_SLOT;
    Slot* slot = NULL;
    while (1) {
        Item* item = VCAST(Item*, *iitem_to_add);
        slot = inventorySearchItem(inventory, item->id, from_slot, &next_free);
        if (slot == NULL) {
            break;
        }
        IItem* slot_iitem = inventorySlotGetItem(slot);
        Item* slot_item = VCAST(Item*, *slot_iitem);
        // Has space?
        if (slot_item->stack_size < slot_item->max_stack_size) {
            const int stack_left = slot_item->max_stack_size - slot_item->stack_size;
            // Can fit into stack?
            if (stack_left >= item->stack_size) {
                slot_item->stack_size += item->stack_size;
                VCALL(*slot_iitem, destroy);
                itemDestroy(slot_iitem);
                return INVENTORY_STORE_RESULT_ADDED_ALL;
            }
            slot_item->stack_size = slot_item->max_stack_size;
            item->stack_size = item->stack_size - stack_left;
            exit_code = INVENTORY_STORE_RESULT_ADDED_SOME;
        } else {
            exit_code = INVENTORY_STORE_RESULT_NO_SPACE;
        }
        from_slot = slot->index + 1;
        if (persisted_next_free == INVENTORY_NO_FREE_SLOT) {
            // Keep the first free slot we found and don't update
            // it later since we will have stated searching from
            // a different offset and thus will not be the first
            // free slot
            persisted_next_free = next_free;
        }
        next_free = INVENTORY_NO_FREE_SLOT;
    }
    if (persisted_next_free != INVENTORY_NO_FREE_SLOT) {
        slot = &inventory->slots[persisted_next_free];
    } else {
        slot = inventoryFindFreeSlot(inventory, 0);
        if (slot == NULL) {
            return exit_code;
        }
    }
    VCALL_SUPER(*iitem_to_add, Renderable, applyInventoryRenderAttributes);
    if (inventorySlotIsRef(slot)) {
        slot->data.ref->data.item = iitem_to_add;
    } else {
        slot->data.item = iitem_to_add;
    }
    return INVENTORY_STORE_RESULT_ADDED_NEW_SLOT;
}

void inventoryLoadTexture(VSelf) __attribute__((alias("Inventory_loadTexture")));
void Inventory_loadTexture(VSelf) {

}

void inventoryFreeTexture(VSelf) __attribute__((alias("Inventory_freeTexture")));
void Inventory_freeTexture(VSelf) {

}

#define createSlot(offset, _index, name) ({ \
    Slot* slot = &inventory->slots[offset + _index]; \
    slot->index = offset + _index; \
    slot->data.item = NULL; \
    slot->position = PLAYER_INV_##name##_POS; \
    slot->dimensions = INV_SLOT_DIMS; \
    slot->blocked = false; \
})

void initArmorSlots(Inventory* inventory) {
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 0, ARMOR_HELMET);
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 1, ARMOR_CHESTPLATE);
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 2, ARMOR_LEGGINGS);
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 3, ARMOR_BOOTS);
}

void initCraftingSlots(Inventory* inventory) {
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 0, CRAFTING_TOP_LEFT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 1, CRAFTING_TOP_RIGHT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 2, CRAFTING_BOTTOM_LEFT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 3, CRAFTING_BOTTOM_RIGHT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 4, CRAFTING_RESULT);
}

void initStorageSlots(Inventory* inventory) {
    for (int i = INVENTORY_SLOT_STORAGE_OFFSET; i < INVENTORY_SLOT_HOTBAR_OFFSET; i++) {
        Slot* slot = &inventory->slots[i];
        slot->data.item = NULL;
        slot->index = i;
        slot->dimensions = INV_SLOT_DIMS;
        slot->blocked = false;
        const uint8_t local_index = i - INVENTORY_SLOT_STORAGE_OFFSET;
        slot->position = playerInvStoragePos(
            local_index % PLAYER_INV_STORAGE_SLOTS_WIDTH,
            local_index / PLAYER_INV_STORAGE_SLOTS_WIDTH
        );
    }
}

void initHotbarSlots(Inventory* inventory) {
    const Hotbar* hotbar = inventory->hotbar;
    for (int i = INVENTORY_SLOT_HOTBAR_OFFSET; i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        const uint8_t hotbar_index = i - INVENTORY_SLOT_HOTBAR_OFFSET;
        slot->data.ref = &hotbar->slots[hotbar_index];
        slot->index = i;
        slot->blocked = false;
        slot->dimensions = INV_SLOT_DIMS;
        slot->position = playerInvHotbarPos(i - INVENTORY_SLOT_HOTBAR_OFFSET);
    }
}

void inventoryInit(Inventory* inventory, Hotbar* hotbar) {
    uiInit(&inventory->ui);
    inventory->hotbar = hotbar;
    inventory->slots = NULL;
    cvector_init(inventory->slots, INVENTORY_SLOT_COUNT, NULL);
    initArmorSlots(inventory);
    initCraftingSlots(inventory);
    initStorageSlots(inventory);
    initHotbarSlots(inventory);
}