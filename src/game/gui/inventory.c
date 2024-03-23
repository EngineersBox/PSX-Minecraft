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
    if (from_slot >= INVENTORY_SLOT_STORAGE_OFFSET) {
        return NULL;
    }
    *next_free = INVENTORY_NO_FREE_SLOT;
    for (uint8_t i = from_slot; i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->item == NULL) {
            if (*next_free == INVENTORY_NO_FREE_SLOT) {
                *next_free = i;
            }
            continue;
        }
        const Item* item = VCAST(Item*, *slot->item);
        if (item->id == id) {
            return slot;
        }
    }
    return NULL;
}

Slot* inventoryFindFreeSlot(const Inventory* inventory, const uint8_t from_slot) {
    if (from_slot >= INVENTORY_SLOT_STORAGE_OFFSET) {
        return NULL;
    }
    for (uint8_t i = from_slot; i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->item == NULL) {
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
inventory_store_item_start:
    Item* item = VCAST(Item*, *iitem_to_add);
    Slot* slot = inventorySearchItem(inventory, item->id, from_slot, &next_free);
    if (slot == NULL) {
        goto inventory_store_item_check_free_space;
    }
    IItem* slot_iitem = slot->item;
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
    next_free = INVENTORY_NO_FREE_SLOT;
    goto inventory_store_item_start;
inventory_store_item_check_free_space:
    if (next_free != INVENTORY_NO_FREE_SLOT) {
        slot = &inventory->slots[next_free];
    } else {
        slot = inventoryFindFreeSlot(inventory, 0);
        if (slot == NULL) {
            return exit_code;
        }
    }
    slot->item = iitem_to_add;
    return INVENTORY_STORE_RESULT_ADDED_NEW_SLOT;
}

void inventoryLoadTexture(VSelf) __attribute__((alias("Inventory_loadTexture")));
void Inventory_loadTexture(VSelf) {

}

void inventoryFreeTexture(VSelf) __attribute__((alias("Inventory_freeTexture")));
void Inventory_freeTexture(VSelf) {

}

#define createSlot(offset, _index, name) ({\
    cvector_push_back(inventory->slots, (Slot) {}); \
    Slot* slot = &inventory->slots[offset + _index]; \
    slot->index = offset + _index; \
    slot->item = NULL; \
    slot->position = PLAYER_INV_##name##_POS; \
    slot->dimensions = INV_SLOT_DIMS; \
})

inline void initArmorSlots(Inventory* inventory) {
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 0, ARMOR_HELMET);
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 1, ARMOR_CHESTPLATE);
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 2, ARMOR_LEGGINGS);
    createSlot(INVENTORY_SLOT_ARMOR_OFFSET, 3, ARMOR_BOOTS);
}

inline void initCraftingSlots(Inventory* inventory) {
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 0, CRAFTING_TOP_LEFT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 1, CRAFTING_TOP_RIGHT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 2, CRAFTING_BOTTOM_LEFT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 3, CRAFTING_BOTTOM_RIGHT);
    createSlot(INVENTORY_SLOT_CRAFTING_OFFSET, 4, CRAFTING_RESULT);
}

inline void initStorageSlots(Inventory* inventory) {
    for (int i = INVENTORY_SLOT_STORAGE_OFFSET; i < INVENTORY_SLOT_HOTBAR_OFFSET; i++) {
        const uint8_t local_index = i - INVENTORY_SLOT_STORAGE_OFFSET;
        cvector_push_back(inventory->slots, (Slot) {});
        Slot* slot = &inventory->slots[i];
        slot->item = NULL;
        slot->index = i;
        slot->dimensions = INV_SLOT_DIMS;
        slot->position = playerInvStoragePos(
            local_index % PLAYER_INV_STORAGE_SLOTS_WIDTH,
            local_index / PLAYER_INV_STORAGE_SLOTS_WIDTH
        );
    }
}

inline void initHotbarSlots(Inventory* inventory) {
    for (int i = INVENTORY_SLOT_HOTBAR_OFFSET; i < INVENTORY_SLOT_COUNT; i++) {
        cvector_push_back(inventory->slots, (Slot) {});
        Slot* slot = &inventory->slots[i];
        slot->item = NULL;
        slot->index = i;
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
    initArmorSlots(inventory);
    initHotbarSlots(inventory);
}