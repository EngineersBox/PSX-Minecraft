#include "utils.h"

#include "../items/items.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/components/cursor.h"
#include "slot.h"

void cursorSplitOrStoreOne(Slot* slot,
                           SlotItemGetter getter,
                           SlotItemSetter setter) {
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
            cursor.held_data = slot_iitem;
            setter(slot, NULL);
            return;
        }
        // Do the normal splitting of stacks between the
        // existing slot stack and a new stack
        IItem* split_stack = itemGetConstructor(item->id)();
        assert(split_stack);
        Item* split_stack_item = VCAST_PTR(Item*, split_stack);
        split_stack_item->stack_size = item->stack_size >> 1;
        itemSetWorldState(split_stack_item, false);
        cursor.held_data = split_stack;
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
        IItem* new_slot_iitem = itemGetConstructor(held_item->id)();
        assert(new_slot_iitem);
        Item* new_slot_item = VCAST_PTR(Item*, new_slot_iitem);
        itemSetWorldState(new_slot_item, true);
        new_slot_item->stack_size = 1;
        setter(slot, new_slot_iitem);
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

void cursorInteractSlot(Slot* slot,
                        SlotItemGetter getter,
                        SlotItemSetter setter) {
    IItem* held_item = (IItem*) cursor.held_data;
    IItem* slot_item = getter(slot);
    setter(slot, held_item);
    cursor.held_data = slot_item;
}
