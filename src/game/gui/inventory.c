#include "inventory.h"

#include <interface99.h>

#include "../../ui/components/background.h"
#include "../../util/interface99_extensions.h"

const char* INVENTORY_STORE_RESULT_NAMES[] = {
    MK_INVENTORY_STORE_RESULT_LSIT(P99_STRING_ARRAY_INDEX)
};

#define createSlot(offset, _index, name) ({ \
    cvector_push_back(inventory->slots, (Slot){}); \
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
        cvector_push_back(inventory->slots, (Slot){});
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
        cvector_push_back(inventory->slots, (Slot){});
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
    IUIComponent* component = uiAddComponent(&inventory->ui);
    UIBackground* background = (UIBackground*) malloc(sizeof(UIBackground));
    background->component.position = (DVECTOR) {
        .vx = CENTRE_X - (INVENTORY_WIDTH / 2),
        .vy = CENTRE_Y - (INVENTORY_HEIGHT / 2)
    };
    background->component.dimensions = (DVECTOR) {
        .vx = INVENTORY_WIDTH,
        .vy = INVENTORY_HEIGHT
    };
    background->texture_coords = (DVECTOR) {
        .vx = 0,
        .vy = 82
    };
    background->texture_width = (DVECTOR) {
        .vx = INVENTORY_WIDTH,
        .vy = INVENTORY_HEIGHT
    };
    background->texture = NULL;
    DYN_PTR(component, UIBackground, IUIComponent, background);
    inventory->hotbar = hotbar;
    inventory->slots = NULL;
    cvector_init(inventory->slots, 0, NULL);
    initArmorSlots(inventory);
    initCraftingSlots(inventory);
    initStorageSlots(inventory);
    initHotbarSlots(inventory);
}

void inventoryRenderSlots(const Inventory* inventory, RenderContext* ctx, Transforms* transforms) {
    if (!inventory->ui.active) {
        return;
    }
    for (int i = 0; i < INVENTORY_SLOT_HOTBAR_OFFSET; i++) {
        const Slot* slot = &inventory->slots[i];
        if (slot->data.item == NULL) {
            continue;
        }
        Item* item = VCAST_PTR(Item*,slot->data.item);
        item->position.vx = slot->position.vx;
        item->position.vy = slot->position.vy;
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    for (int i = INVENTORY_SLOT_HOTBAR_OFFSET; i < INVENTORY_SLOT_COUNT; i++) {
        const Slot* slot = &inventory->slots[i];
        if (slot->data.ref == NULL || slot->data.ref->data.item == NULL) {
            continue;
        }
        Item* item = VCAST_PTR(Item*, slot->data.ref->data.item);
        VECTOR prev_position = item->position;
        item->position.vx = slot->position.vx;
        item->position.vy = slot->position.vy;
        VCALL_SUPER(*slot->data.ref->data.item, Renderable, renderInventory, ctx, transforms);
        item->position = prev_position;
    }
}

Slot* inventorySearchItem(const Inventory* inventory, const ItemID id, const uint8_t from_slot, uint8_t* next_free) {
    *next_free = INVENTORY_NO_FREE_SLOT;
    if (from_slot >= INVENTORY_SLOT_COUNT) {
        return NULL;
    }
    // Hotbar first
    for (uint8_t i = max(from_slot, INVENTORY_SLOT_HOTBAR_OFFSET); i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        const Slot* slot_ref = slot->data.ref;
        if (slot_ref->data.item == NULL) {
            if (*next_free == INVENTORY_NO_FREE_SLOT) {
                *next_free = i;
            }
            continue;
        }
        const Item* item = VCAST_PTR(Item*, slot_ref->data.item);
        if (item->id == id) {
            return slot;
        }
    }
    // Inventory storage second
    for (uint8_t i = from_slot; i < INVENTORY_SLOT_HOTBAR_OFFSET; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->data.item == NULL) {
            if (*next_free == INVENTORY_NO_FREE_SLOT) {
                *next_free = i;
            }
            continue;
        }
        const Item* item = VCAST_PTR(Item*, slot->data.item);
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
    // Hotbar first
    for (uint8_t i = max(from_slot, INVENTORY_SLOT_HOTBAR_OFFSET); i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->data.ref->data.item == NULL) {
            return slot;
        }
    }
    // Inventory storage second
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
    //       1_1. [ii:TRUE] Decrement stack size to residual and go to 1
    //       1_2: [ii:FALSE] Done
    //     iii. [a:FALSE] Go to 2
    //   b. [1:FALSE] Go to 2
    // 2. Is there space in the inventory
    //   a. [2:TRUE] Add the item stack into the next free slot
    //   b. [2:FALSE] Add item back into array at the same index (make sure item world position is correct)
    InventoryStoreResult exit_code = INVENTORY_STORE_RESULT_ADDED_ALL;
    uint8_t from_slot = INVENTORY_SLOT_STORAGE_OFFSET;
    uint8_t next_free = INVENTORY_NO_FREE_SLOT;
    uint8_t persisted_next_free = INVENTORY_NO_FREE_SLOT;
    Slot* slot = NULL;
    while (1) {
        Item* item = VCAST_PTR(Item*, iitem);
        slot = inventorySearchItem(inventory, item->id, from_slot, &next_free);
        if (slot == NULL) {
            break;
        }
        const IItem* slot_iitem = inventorySlotGetItem(slot);
        Item* slot_item = VCAST_PTR(Item*, slot_iitem);
        // Has space?
        if (slot_item->stack_size < slot_item->max_stack_size) {
            const int stack_left = slot_item->max_stack_size - slot_item->stack_size;
            // Can fit into stack?
            if (stack_left >= item->stack_size) {
                slot_item->stack_size += item->stack_size;
                // Don't free or destroy the iitem_to_add as it was passed
                // as context to this function. It's up to the caller to
                // handle it depending on the state enum returned.
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
            // it later since we will have started searching from
            // a different offset and thus will not be the first
            // free slot.
            persisted_next_free = next_free;
        }
        next_free = INVENTORY_NO_FREE_SLOT;
    }
    if (persisted_next_free != INVENTORY_NO_FREE_SLOT) {
        // If we have already found a free spot during the search
        // invocations from before, we can use that to avoid
        // unnecessary search operations.
        slot = &inventory->slots[persisted_next_free];
    } else {
        slot = inventoryFindFreeSlot(inventory, 0);
        if (slot == NULL) {
            return exit_code;
        }
    }
    VCALL_SUPER(*iitem, Renderable, applyInventoryRenderAttributes);
    inventorySlotSetItem(slot, iitem);
    return INVENTORY_STORE_RESULT_ADDED_NEW_SLOT;
}

#define GUI_BUNDLE_NAME "gui"
#define INVENTORY_TEXTURE_NAME "inventory"

void inventoryOpen(VSelf) __attribute__((alias("Inventory_open")));
void Inventory_open(VSelf) {
    VSELF(Inventory);
    if (self->ui.active) {
        return;
    }
    self->ui.active = true;
    const UIBackground* background = VCAST(UIBackground*, self->ui.components[0]);
    if (assetLoadTextureDirect(GUI_BUNDLE_NAME, INVENTORY_TEXTURE_NAME, background->texture)) {
        printf("[INVENTORY] Failed to load texture\n");
    }
}

void inventoryClose(VSelf) __attribute__((alias("Inventory_close")));
void Inventory_close(VSelf) {
    VSELF(Inventory);
    if (!self->ui.active) {
        return;
    }
    self->ui.active = false;
    UIBackground* background = VCAST(UIBackground*, self->ui.components[0]);
    background->texture = NULL;
}

bool inventoryInputHandler(const Input* input, void* ctx) {
    Inventory* inventory = (Inventory*) ctx;
    if (isPressed(input->pad, PAD_L1)) {
        if (inventory->ui.active) {
            inventoryClose(inventory);
            return false;
        }
        inventoryOpen(inventory);
        return true;
    }
    return inventory->ui.active;
}

void inventoryRegisterInputHandler(VSelf, Input* input) __attribute__((alias("Inventory_registerInputHandler")));
void Inventory_registerInputHandler(VSelf, Input* input) {
    VSELF(Inventory);
    const ContextualInputHandler handler = (ContextualInputHandler) {
        .ctx = self,
        .input_handler = inventoryInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}
