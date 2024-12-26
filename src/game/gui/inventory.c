#include "inventory.h"

#include <interface99.h>

#include "../../logging/logging.h"
#include "../items/items.h"
#include "../../ui/components/background.h"
#include "../../util/interface99_extensions.h"
#include "psxpad.h"
#include "slot.h"
#include "../../ui/components/cursor.h"
#include "../world/world_structure.h"

FWD_DECL void worldDropItemStack(World* world, IItem* item, const u8 count);

/*const char* INVENTORY_STORE_RESULT_NAMES[] = {*/
/*    MK_INVENTORY_STORE_RESULT_LIST(P99_STRING_ARRAY_INDEX)*/
/*};*/

void inventoryInit(Inventory* inventory, Hotbar* hotbar) {
    uiInit(&inventory->ui);
    IUIComponent* component = uiAddComponent(&inventory->ui);
    UIBackground* background = (UIBackground*) malloc(sizeof(UIBackground));
    background->component.position = (DVECTOR) {
        .vx = CENTRE_X - (INVENTORY_WIDTH >> 1),
        .vy = CENTRE_Y - (INVENTORY_HEIGHT >> 1)
    };
    background->component.dimensions = (DVECTOR) {
        .vx = INVENTORY_WIDTH,
        .vy = INVENTORY_HEIGHT
    };
    background->texture_coords = (DVECTOR) {
        .vx = 0,
        .vy = 0
    };
    background->texture_width = (DVECTOR) {
        .vx = INVENTORY_WIDTH,
        .vy = INVENTORY_HEIGHT
    };
    background->texture = (Texture) {0};
    DYN_PTR(component, UIBackground, IUIComponent, background);
    inventory->hotbar = hotbar;
    for (u8 y = 0; y < slotGroupDim(INVENTORY_ARMOUR, Y); y++) {
        for (u8 x = 0; x < slotGroupDim(INVENTORY_ARMOUR, X); x++) {
            createSlot(inventory->slots, INVENTORY_ARMOUR, x, y);
        }
    }
    for (u8 y = 0; y < slotGroupDim(INVENTORY_CRAFTING, Y); y++) {
        for (u8 x = 0; x < slotGroupDim(INVENTORY_CRAFTING, X); x++) {
            createSlot(inventory->slots, INVENTORY_CRAFTING, x, y);
        }
    }
    createSlot(inventory->slots, INVENTORY_CRAFTING_RESULT, 0, 0);
    for (u8 y = 0; y < slotGroupDim(INVENTORY_MAIN, Y); y++) {
        for (u8 x = 0; x < slotGroupDim(INVENTORY_MAIN, X); x++) {
            createSlot(inventory->slots, INVENTORY_MAIN, x, y);
        }
    }
    for (u8 y = 0; y < slotGroupDim(INVENTORY_HOTBAR, Y); y++) {
        for (u8 x = 0; x < slotGroupDim(INVENTORY_HOTBAR, X); x++) {
            createSlotRef(
                inventory->slots,
                INVENTORY_HOTBAR,
                x, y,
                hotbar->slots,
                HOTBAR,
                x, y
            );
        }
    }
    inventory->debounce = 0;
}

void inventoryRenderSlots(const Inventory* inventory,
                          InventorySlotGroups groups,
                          RenderContext* ctx,
                          Transforms* transforms) {
    if (groups == INVENTORY_SLOT_GROUP_NONE) {
        return;
    }
    if (groups & INVENTORY_SLOT_GROUP_ARMOUR) {
        for (u8 y = 0; y < slotGroupDim(INVENTORY_ARMOUR, Y); y++) {
            const u8 y_offset = slotGroupDim(INVENTORY_ARMOUR, X) * y;
            for (u8 x = 0; x < slotGroupDim(INVENTORY_ARMOUR, X); x++) {
                const u8 i = slotGroupIndexOffset(INVENTORY_ARMOUR) + y_offset + x;
                const Slot* slot = &inventory->slots[i];
                if (slot->data.item == NULL) {
                    continue;
                }
                Item* item = VCAST_PTR(Item*,slot->data.item);
                item->position.vx = slotGroupScreenPosition(INVENTORY_ARMOUR, X, x);
                item->position.vy = slotGroupScreenPosition(INVENTORY_ARMOUR, Y, y);
                VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
            }
        }
    }
    if (groups & INVENTORY_SLOT_GROUP_CRAFTING) {
        for (u8 y = 0; y < slotGroupDim(INVENTORY_CRAFTING, Y); y++) {
            const u8 y_offset = slotGroupDim(INVENTORY_CRAFTING, X) * y;
            for (u8 x = 0; x < slotGroupDim(INVENTORY_CRAFTING, X); x++) {
                const u8 i = slotGroupIndexOffset(INVENTORY_CRAFTING) + y_offset + x;
                const Slot* slot = &inventory->slots[i];
                if (slot->data.item == NULL) {
                    continue;
                }
                Item* item = VCAST_PTR(Item*,slot->data.item);
                item->position.vx = slotGroupScreenPosition(INVENTORY_CRAFTING, X, x);
                item->position.vy = slotGroupScreenPosition(INVENTORY_CRAFTING, Y, y);
                VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
            }
        }
    }
    if (groups & INVENTORY_SLOT_GROUP_CRAFTING_RESULT) {
        const u8 i = slotGroupIndexOffset(INVENTORY_CRAFTING_RESULT);
        const Slot* slot = &inventory->slots[i];
        if (slot->data.item != NULL) {
            Item* item = VCAST_PTR(Item*, slot->data.item);
            item->position.vx = slotGroupScreenPosition(INVENTORY_CRAFTING_RESULT, X, 0);
            item->position.vy = slotGroupScreenPosition(INVENTORY_CRAFTING_RESULT, Y, 0);
            VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
        }
    }
    if (groups & INVENTORY_SLOT_GROUP_MAIN) {
        for (u8 y = 0; y < slotGroupDim(INVENTORY_MAIN, Y); y++) {
            const u8 y_offset = slotGroupDim(INVENTORY_MAIN, X) * y;
            for (u8 x = 0; x < slotGroupDim(INVENTORY_MAIN, X); x++) {
                const u8 i = slotGroupIndexOffset(INVENTORY_MAIN) + y_offset + x;
                const Slot* slot = &inventory->slots[i];
                if (slot->data.item == NULL) {
                    continue;
                }
                Item* item = VCAST_PTR(Item*,slot->data.item);
                item->position.vx = slotGroupScreenPosition(INVENTORY_MAIN, X, x);
                item->position.vy = slotGroupScreenPosition(INVENTORY_MAIN, Y, y);
                VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
            }
        }
    }
    if (groups & INVENTORY_SLOT_GROUP_HOTBAR) {
        for (u8 y = 0; y < slotGroupDim(INVENTORY_HOTBAR, Y); y++) {
            const u8 y_offset = slotGroupDim(INVENTORY_HOTBAR, X) * y;
            for (u8 x = 0; x < slotGroupDim(INVENTORY_HOTBAR, X); x++) {
                const u8 i = slotGroupIndexOffset(INVENTORY_HOTBAR) + y_offset + x;
                const Slot* slot = &inventory->slots[i];
                if (slot->data.ref == NULL || slot->data.ref->data.item == NULL) {
                    continue;
                }
                Item* item = VCAST_PTR(Item*, slot->data.ref->data.item);
                VECTOR prev_position = item->position;
                item->position.vx = slotGroupScreenPosition(INVENTORY_HOTBAR, X, x);
                item->position.vy = slotGroupScreenPosition(INVENTORY_HOTBAR, Y, y);
                VCALL_SUPER(*slot->data.ref->data.item, Renderable, renderInventory, ctx, transforms);
                item->position = prev_position;
            }
        }
    }
}

Slot* inventorySearchItem(Inventory* inventory,
                          const ItemID id,
                          const u8 from_slot,
                          u8* next_free) {
    *next_free = INVENTORY_NO_FREE_SLOT;
    if (from_slot >= INVENTORY_SLOT_COUNT) {
        return NULL;
    }
    // Hotbar first
    for (u8 i = max(from_slot, slotGroupIndexOffset(INVENTORY_HOTBAR)); i < INVENTORY_SLOT_COUNT; i++) {
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
    for (u8 i = from_slot; i < slotGroupIndexOffset(INVENTORY_HOTBAR); i++) {
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

Slot* inventoryFindFreeSlot(Inventory* inventory, const u8 from_slot) {
    if (from_slot >= INVENTORY_SLOT_COUNT) {
        return NULL;
    }
    // Hotbar first
    for (u8 i = max(from_slot, slotGroupIndexOffset(INVENTORY_HOTBAR)); i < INVENTORY_SLOT_COUNT; i++) {
        Slot* slot = &inventory->slots[i];
        if (slot->data.ref->data.item == NULL) {
            return slot;
        }
    }
    // Inventory storage second
    for (u8 i = from_slot; i < slotGroupIndexOffset(INVENTORY_HOTBAR); i++) {
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
    u8 from_slot = slotGroupIndexOffset(INVENTORY_MAIN);
    u8 next_free = INVENTORY_NO_FREE_SLOT;
    u8 persisted_next_free = INVENTORY_NO_FREE_SLOT;
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
        if (slot_item->stack_size < itemGetMaxStackSize(slot_item->id)) {
            const int stack_left = itemGetMaxStackSize(slot_item->id) - slot_item->stack_size;
            // Can fit into stack?
            if (stack_left >= item->stack_size) {
                slot_item->stack_size += item->stack_size;
                // Don't free or destroy the iitem_to_add as it was passed
                // as context to this function. It's up to the caller to
                // handle it depending on the state enum returned.
                return INVENTORY_STORE_RESULT_ADDED_ALL;
            }
            slot_item->stack_size = itemGetMaxStackSize(slot_item->id);
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

void inventoryOpen(VSelf) ALIAS("Inventory_open");
void Inventory_open(VSelf) {
    VSELF(Inventory);
    if (self->ui.active) {
        return;
    }
    self->ui.active = true;
    UIBackground* background = VCAST(UIBackground*, self->ui.components[0]);
    assetLoadTextureDirect(
        ASSET_BUNDLE__GUI,
        ASSET_TEXTURE__GUI__INVENTORY,
        &background->texture
    );
}

void inventoryClose(VSelf) ALIAS("Inventory_close");
void Inventory_close(VSelf) {
    VSELF(Inventory);
    if (!self->ui.active) {
        return;
    }
    self->ui.active = false;
    UIBackground* background = VCAST(UIBackground*, self->ui.components[0]);
    background->texture = (Texture) {0};
}

static bool debounce(Inventory* inventory) {
    if (time_ms - inventory->debounce >= INVENTORY_DEBOUNCE_MS) {
        inventory->debounce = time_ms;
        return true;
    }
    return false;
}

bool inventoryInputHandler(const Input* input, void* ctx) {
    Inventory* inventory = (Inventory*) ctx;
    if (isPressed(input->pad, BINDING_OPEN_INVENTORY) && debounce(inventory)) {
        if (inventory->ui.active) {
            inventoryClose(inventory);
            return false;
        }
        inventoryOpen(inventory);
        return true;
    }
    inventoryCursorHandler(
        inventory,
        INVENTORY_SLOT_GROUP_ALL,
        input,
        ctx
    );
    return inventory->ui.active;
}

void inventoryRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("Inventory_registerInputHandler");
void Inventory_registerInputHandler(VSelf, Input* input, void* ctx) {
    VSELF(Inventory);
    const InputHandlerVTable handler = (InputHandlerVTable) {
        .ctx = self,
        .input_handler = inventoryInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}

void cursorInteractSlot(Slot* slot) {
    IItem* held_item = (IItem*) cursor.held_data;
    IItem* slot_item = inventorySlotGetItem(slot);
    inventorySlotSetItem(slot, held_item);
    cursor.held_data = slot_item;
}



void cursorSplitOrStoreOne(Slot* slot) {
    IItem* held_iitem = (IItem*) cursor.held_data;
    IItem* slot_iitem = inventorySlotGetItem(slot);
    if (held_iitem == NULL) {
        // Split targetted slot stack
        if (slot_iitem == NULL) {
            // Nothing to stire
            return;
        }
        Item* item = VCAST_PTR(Item*, slot_iitem);
        if (item->stack_size == 1) {
            // Single item, just move the stack to avoid
            // creating a new IItem and copying the held
            // item data then deleting the held item
            cursor.held_data = slot_iitem;
            inventorySlotSetItem(slot, NULL);
            return;
        }
        // Do the normal splitting of stacks between the
        // existing slot stack and a new stack
        IItem* split_stack = itemGetConstructor(item->id)();
        assert(split_stack);
        Item* split_stack_item = VCAST_PTR(Item*, split_stack);
        split_stack_item->stack_size = item->stack_size >> 1;
        itemSetWorldState(split_stack_item, false);
        item->stack_size -= split_stack_item->stack_size;
        return;
    }
    Item* slot_item = VCAST_PTR(Item*, slot_iitem);
    Item* held_item = VCAST_PTR(Item*, held_iitem);
    if (slot_iitem == NULL) {
        // No items in targetted slot, store one
        if (held_item->stack_size == 1) {
            // Single item, just move the stack to avoid
            // creating a new IItem and copying the held
            // item data then deleting the held item
            inventorySlotSetItem(slot, held_iitem);
            return;
        }
        // Create a new item with a stack size of 1
        IItem* new_slot_iitem = itemGetConstructor(held_item->id)();
        assert(new_slot_iitem);
        Item* new_slot_item = VCAST_PTR(Item*, new_slot_iitem);
        itemSetWorldState(new_slot_item, true);
        new_slot_item->stack_size = 1;
    } else if (held_item->id != slot_item->id) {
        // Can't override an existing item in the slot
        // that doesn't match
        return;
    }
    slot_item->stack_size++;
    held_item->stack_size--;
    if (held_item->stack_size == 0) {
        VCALL(*held_iitem, destroy);
        cursor.held_data = NULL;
    }
}

void cursorHandler(Inventory* inventory,
                   InventorySlotGroups groups,
                   bool split_or_store_one) {
    if (!quadIntersectLiteral(
        &cursor.component.position,
        CENTRE_X - (INVENTORY_WIDTH >> 1),
        CENTRE_Y - (INVENTORY_HEIGHT >> 1),
        INVENTORY_WIDTH,
        INVENTORY_HEIGHT
    )) {
        worldDropItemStack(
            world,
            cursor.held_data,
            0
        );
        return;
    }
    Slot* slot = NULL;
    if (groups & INVENTORY_SLOT_GROUP_ARMOUR && slotGroupIntersect(
        INVENTORY_ARMOUR,
        &cursor.component.position
    )) {
        slot = &inventory->slots[slotGroupCursorSlot(
            INVENTORY_ARMOUR,
            &cursor.component.position
        )];
    } else if (groups & INVENTORY_SLOT_GROUP_CRAFTING && slotGroupIntersect(
        INVENTORY_CRAFTING,
        &cursor.component.position
    )) {
        slot = &inventory->slots[slotGroupCursorSlot(
            INVENTORY_CRAFTING,
            &cursor.component.position
        )];
    } else if (groups & INVENTORY_SLOT_GROUP_CRAFTING_RESULT && slotGroupIntersect(
        INVENTORY_CRAFTING_RESULT,
        &cursor.component.position
    )) {
        slot = &inventory->slots[slotGroupCursorSlot(
            INVENTORY_CRAFTING_RESULT,
            &cursor.component.position
        )];
    } else if (groups & INVENTORY_SLOT_GROUP_MAIN && slotGroupIntersect(
        INVENTORY_MAIN,
        &cursor.component.position
    )) {
        slot = &inventory->slots[slotGroupCursorSlot(
            INVENTORY_MAIN,
            &cursor.component.position
        )];
    } else if (groups & INVENTORY_SLOT_GROUP_HOTBAR && slotGroupIntersect(
        INVENTORY_HOTBAR,
        &cursor.component.position
    )) {
        slot = &inventory->slots[slotGroupCursorSlot(
            INVENTORY_HOTBAR,
            &cursor.component.position
        )];
    } else {
        return;
    }
    split_or_store_one ? cursorSplitOrStoreOne(slot) : cursorInteractSlot(slot);
}

void inventoryCursorHandler(Inventory* inventory,
                            InventorySlotGroups groups,
                            const Input* input,
                            void* ctx) {
    const PADTYPE* pad = input->pad;
    if (isPressed(pad, BINDING_CURSOR_CLICK) && debounce(inventory)) {
        cursorHandler(inventory, groups, false);
    } else if (isPressed(pad, BINDING_DROP_ITEM) && cursor.held_data != NULL) {
        worldDropItemStack(
            world,
            (IItem*) cursor.held_data,
            0
        );
    } else if (isPressed(pad, BINDING_SPLIT_OR_STORE_ONE)) {
        cursorHandler(inventory, groups, false);
    }
}
