#include "player.h"

#include "../util/interface99_extensions.h"

void playerInit(Player* player) {
    Inventory* inventory = (Inventory*) malloc(sizeof(Inventory));
    Hotbar* hotbar = (Hotbar*) malloc(sizeof(Hotbar));
    hotbarInit(hotbar);
    inventoryInit(inventory, hotbar);
    DYN_PTR(&player->hotbar, Hotbar, IUI, hotbar);
    DYN_PTR(&player->inventory, Inventory, IUI, inventory);
}

void playerDestroy(const Player* player) {
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    free(inventory);
    free(hotbar);
}

void playerRender(const Player* player, RenderContext* ctx, Transforms* transforms) {
    hotbarRenderSlots(VCAST(const Hotbar*, player->hotbar), ctx, transforms);
    uiRender(VCAST(const UI*, player->hotbar), ctx, transforms);
    uiRender(VCAST(const UI*, player->inventory), ctx, transforms);
}

PlayerStoreResult playerStoreItem(Player* player, IItem* iitem) {
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
    const Inventory* inventory = VCAST(Inventory*, player->inventory);
    PlayerStoreResult exit_code = PLAYER_STORE_RESULT_ADDED_ALL;
    IItem* iitem_to_add = iitem;
    uint8_t from_slot = INVENTORY_SLOT_STORAGE_OFFSET;
    start:
        Item* item = VCAST(Item*, *iitem_to_add);
    Slot* slot = inventorySearchItem(inventory, item->id, from_slot);
    if (slot == NULL) {
        goto check_free_space;
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
            return PLAYER_STORE_RESULT_ADDED_ALL;
        }
        slot_item->stack_size = slot_item->max_stack_size;
        item->stack_size = item->stack_size - stack_left;
        exit_code = PLAYER_STORE_RESULT_ADDED_SOME;
    } else {
        exit_code = PLAYER_STORE_RESULT_NO_SPACE;
    }
    from_slot = slot->index;
    goto start;
    check_free_space:
        slot = inventoryFindFreeSlot(inventory, 0);
    if (slot == NULL) {
        return exit_code;
    }
    slot->item = iitem_to_add;
    return PLAYER_STORE_RESULT_ADDED_NEW_SLOT;
}