#include "utils.h"

#include "../items/items.h"
#include "../../util/interface99_extensions.h"
#include "../../util/debounce.h"
#include "../../ui/components/cursor.h"
#include "slot.h"

static Timestamp cursor_debounce = 0;

void cursorSplitOrStoreOne(Slot* slot,
                           SlotItemGetter getter,
                           SlotItemSetter setter) {
    if (!debounce(&cursor_debounce, CURSOR_DEBOUNCE_MS)) {
        return;
    }
    IItem* held_iitem = (IItem*) cursor.held_data;
    IItem* slot_iitem = getter(slot);
    if (held_iitem == NULL) {
        // Split targetted slot stack
        if (slot_iitem == NULL) {
            // Nothing to store
            return;
        }
        Item* item = VCAST_PTR(Item*, slot_iitem);
        if (item->stack_size == 1) {
            // Single item, just move the stack to avoid
            // creating a new IItem and copying the held
            // item data then deleting the held item
            uiCursorSetHeldData(&cursor, slot_iitem);
            setter(slot, NULL);
            return;
        }
        // Do the normal splitting of stacks between the
        // existing slot stack and a new stack
        IItem* split_stack = itemGetConstructor(item->id)(item->metadata_id);
        assert(split_stack != NULL);
        Item* split_stack_item = VCAST_PTR(Item*, split_stack);
        split_stack_item->stack_size = item->stack_size >> 1;
        // Force applying non-in world state in following call
        split_stack_item->in_world = true;
        itemSetWorldState(split_stack_item, false);
        VCALL_SUPER(*split_stack, Renderable, applyInventoryRenderAttributes);
        uiCursorSetHeldData(&cursor, split_stack);
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
            setter(slot, held_iitem);
            return;
        }
        // Create a new item with a stack size of 1
        IItem* new_slot_iitem = itemGetConstructor(held_item->id)(held_item->metadata_id);
        assert(new_slot_iitem != NULL);
        Item* new_slot_item = VCAST_PTR(Item*, new_slot_iitem);
        // Force applying non-in world state in following call
        new_slot_item->in_world = true;
        itemSetWorldState(new_slot_item, false);
        VCALL_SUPER(*new_slot_iitem, Renderable, applyInventoryRenderAttributes);
        new_slot_item->stack_size = 1;
        setter(slot, new_slot_iitem);
        held_item->stack_size--;
        return;
    } else if (!itemEquals(held_item, slot_item) || slot_item->stack_size == itemGetMaxStackSize(slot_item->id)) {
        // Can't override an existing item in the slot
        // that doesn't match or we cant add an item to
        // an already full stack
        return;
    }
    slot_item->stack_size++;
    held_item->stack_size--;
    if (held_item->stack_size == 0) {
        VCALL(*held_iitem, destroy);
        uiCursorSetHeldData(&cursor, NULL);
    }
}

void cursorInteractSlot(Slot* slot,
                        SlotItemGetter getter,
                        SlotItemSetter setter) {
    if (!debounce(&cursor_debounce, CURSOR_DEBOUNCE_MS)) {
        return;
    }
    IItem* held_iitem = (IItem*) cursor.held_data;
    IItem* slot_iitem = getter(slot);
    if (slot_iitem == NULL || held_iitem == NULL) {
        setter(slot, held_iitem);
        uiCursorSetHeldData(&cursor, slot_iitem);
        return;
    }
    Item* slot_item = VCAST_PTR(Item*, slot_iitem);
    Item* held_item = VCAST_PTR(Item*, held_iitem);
    const u8 stack_left = itemGetMaxStackSize(slot_item->id) - slot_item->stack_size;
    if (!itemEquals(held_item, slot_item) || stack_left == 0) {
        return;
    }
    const u8 held_assignable = min(stack_left, held_item->stack_size);
    slot_item->stack_size += held_assignable;
    if (held_assignable == held_item->stack_size) {
        VCALL(*held_iitem, destroy);
        uiCursorSetHeldData(&cursor, NULL);
    }
}
