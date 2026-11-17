#include "slot.h"

#include <interface99.h>

#include "../../util/interface99_extensions.h"

bool slotDirectHasItem(const Slot* slot) {
    return slot->data.item != NULL;
}

IItem* slotDirectItemGetter(Slot* slot) {
    return slot->data.item;
}

void slotDirectItemSetter(Slot* slot, IItem* item) {
    slot->data.item = item;
}

bool slotRefHasItem(const Slot *slot) {
    return slot->data.ref != NULL && slot->data.ref->data.item != NULL;
}

IItem* slotRefItemGetter(Slot* slot) {
    return slot->data.ref->data.item;
}

void slotRefItemSetter(Slot* slot, IItem* item) {
    slot->data.ref->data.item = item;
}

Slot* slotFromScreenPosition0(const SVECTOR* screen_position,
                              Slot* group_slots,
                              const u16 group_origin_x,
                              const u16 group_origin_y,
                              const u8 group_dim_x,
                              const u8 group_dim_y,
                              const u8 slot_dim_x,
                              const u8 slot_dim_y,
                              const u8 slot_spacing_x,
                              const u8 slot_spacing_y) {
#define groupEnd(dir) (group_origin_##dir + ((group_dim_##dir) * (slot_dim_##dir + slot_spacing_##dir)))
    if (screen_position->vx < group_origin_x || screen_position->vx > groupEnd(x)
        || screen_position->vy < group_origin_y || screen_position->vy > groupEnd(y)) {
        // Outside of the slot group
        return NULL;
    }
#undef groupEnd
    u16 x = screen_position->vx - group_origin_x;
    u16 y = screen_position->vy - group_origin_y;
    // NOTE: This will count the spaces as part of the adjacent slot.
    //       I doubt this will be an issue, but worth noting.
    x /= slot_dim_x + slot_spacing_x;
    y /= slot_dim_y + slot_spacing_y;
    const u8 index = (x * (slot_dim_y)) + y;
    return &group_slots[index];
}

INLINE void slotGroupRender0(RenderContext* ctx,
                             Transforms* transforms,
                             Slot* group_slots,
                             const u16 group_origin_x,
                             const u16 group_origin_y,
                             const u8 group_dim_x,
                             const u8 group_dim_y,
                             const u8 slot_dim_x,
                             const u8 slot_dim_y,
                             const u8 slot_spacing_x,
                             const u8 slot_spacing_y,
                             const u8 y_offset,
                             const u16 group_index_offset,
                             SlotHasItemTester slotHasItem,
                             SlotItemGetter slotItemGetter) {
    for (u8 y = 0; y < group_dim_y; y++) {
        const u8 row = y * group_dim_x;
        const u8 y_pos = group_origin_y + (slot_spacing_y * (y)) + (slot_dim_y * (y)) + y_offset;
        for (u8 x = 0; x < group_dim_x; x++) {
            Slot *slot = &group_slots[group_index_offset + row + x];
            if (!slotHasItem(slot)) continue;
            IItem* iitem = slotItemGetter(slot);
            Item* item = VCAST_PTR(Item*, iitem);
            VECTOR prev_position = slotHasItem == slotRefHasItem ? item->position : VEC3_I32_ZERO;
            item->position.vx = (group_origin_x + (slot_spacing_x * (x)) + (slot_dim_x * (x)));
            item->position.vy = y_pos;
            VCALL_SUPER(*iitem, Renderable, renderInventory, ctx, transforms);
            if (slotHasItem == slotRefHasItem) item->position = prev_position;
        }
    }
}
