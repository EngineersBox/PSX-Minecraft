#include "inventory.h"

#include <interface99.h>
#include "../../util/interface99_extensions.h"

void inventoryRenderSlots(const Inventory* inventory) {
    if (!inventory->ui.active) {
        return;
    }
}

Slot* inventorySearchItem(const Inventory* inventory, const ItemID id, const uint8_t from_slot) {
    if (from_slot >= INVENTORY_SLOT_STORAGE_OFFSET) {
        return NULL;
    }
    for (uint8_t i = from_slot; i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->item == NULL) {
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

void inventoryLoadTexture(VSelf) __attribute__((alias("Inventory_loadTexture")));
void Inventory_loadTexture(VSelf) {

}

void inventoryFreeTexture(VSelf) __attribute__((alias("Inventory_freeTexture")));
void Inventory_freeTexture(VSelf) {

}

void inventoryInit(Inventory* inventory, Hotbar* hotbar) {
    uiInit(&inventory->ui);
    inventory->hotbar = hotbar;
    inventory->slots = NULL;
    cvector_init(inventory->slots, INVENTORY_SLOT_COUNT, NULL);
    for (int i = INVENTORY_SLOT_ARMOR_OFFSET; i < INVENTORY_SLOT_CRAFTING_OFFSET; i++) {
        cvector_push_back(inventory->slots, (Slot) {});
        Slot* slot = &inventory->slots[i];
        slot->index = i;
        slot->
    }
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        cvector_push_back(inventory->slots, (Slot) {});
        Slot* slot = &inventory->slots[i];
        slot->index = i;
        slot->dimensions = INV_SLOT_DIMS;
    }
}