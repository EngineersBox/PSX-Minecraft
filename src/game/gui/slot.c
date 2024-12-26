#include "slot.h"

INLINE IItem* slotDirectItemGetter(Slot* slot) {
    return slot->data.item;
}

INLINE void slotDirectItemSetter(Slot* slot, IItem* item) {
    slot->data.item = item;
}

INLINE IItem* slotRefItemGetter(Slot* slot) {
    return slot->data.ref->data.item;
}

INLINE void slotRefItemSetter(Slot* slot, IItem* item) {
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
    // This will count the spaces as part of the adjacent slot.
    // I doubt this will be an issue, but worth noting.
    x /= slot_dim_x + slot_spacing_x;
    y /= slot_dim_y + slot_spacing_y;
    const u8 index = (x * (slot_dim_y)) + y;
    return &group_slots[index];
}
