#pragma once

#ifndef PSXMC_SLOT_H
#define PSXMC_SLOT_H

#include <stdbool.h>
#include <psxgte.h>
#include <assert.h>
#include <stdint.h>

#include "../items/item.h"
#include "../../util/preprocessor.h"
#include "../../math/math_utils.h"

typedef struct Slot {
    union {
        // Actual data
        IItem* item;
        // Reference to slot elsewhere, i.e. inventory
        // hotbar slots mapping to actual hotbar slots
        struct Slot* ref;
    } data;
    // Slot index starting from top to bottom, left to right on screen
    uint8_t index;
} Slot;

/**
 * Using this type must be paired with the following
 * macro definitions:
 * 
 * #define <name>_SLOT_GROUP_DIMENSIONS_X <u8>
 * #define <name>_SLOT_GROUP_DIMENSIONS_Y <u8>
 * #define <name>_SLOT_GROUP_SLOT_DIMENSIONS_X <u8>
 * #define <name>_SLOT_GROUP_SLOT_DIMENSIONS_Y <u8>
 * #define <name>_SLOT_GROUP_SLOT_SPACING_X <u16>
 * #define <name>_SLOT_GROUP_SLOT_SPACING_Y <u16>
 * #define <name>_SLOT_GROUP_ORIGIN_X <u16>
 * #define <name>_SLOT_GROUP_ORIGIN_Y <u16>
 *
 * And an invocation of a compile time assertion check for
 * sizing via:
 *
 * slotGroupCheck(<name>)
 * */
typedef Slot SlotGroup;

typedef IItem* (*SlotItemGetter)(Slot* slot);
typedef void (*SlotItemSetter)(Slot* slot, IItem* item);

// Utility accessors for slot group definitions
#define slotGroupDim(name, dim) name##_SLOT_GROUP_DIMENSIONS_##dim
#define slotGroupSize(name) (slotGroupDim(name, X) * slotGroupDim(name, Y))
#define slotGroupSlotDim(name, dim) name##_SLOT_GROUP_SLOT_DIMENSIONS_##dim
#define slotGroupSlotSpacing(name, dim) name##_SLOT_GROUP_SLOT_SPACING_##dim
#define slotGroupOrigin(name, dim) name##_SLOT_GROUP_ORIGIN_##dim
#define slotGroupIndexOffset(name) name##_SLOT_GROUP_INDEX_OFFSET
#define slotGroupIntersect(name, pos) quadIntersectLiteral( \
    pos, \
    slotGroupOrigin(name, X), \
    slotGroupOrigin(name, Y), \
    slotGroupDim(name, X), \
    slotGroupDim(name, Y) \
)
#define slotGroupCursorSlot(name, pos) (\
    slotGroupIndexOffset(name) \
    + ((((pos)->vx) - slotGroupOrigin(name, X)) / (slotGroupSlotDim(name, X) + slotGroupSlotSpacing(name, X))) \
    + ( \
        slotGroupDim(name, X) * \
        ((((pos)->vy) - slotGroupOrigin(name, Y)) / (slotGroupSlotDim(name, Y) + slotGroupSlotSpacing(name, Y))) \
    ) \
)

#define slotGroupScreenPosition(name, dim, dim_var) (\
    slotGroupOrigin(name, dim) \
    + (slotGroupSlotSpacing(name, dim) * (dim_var)) \
    + (slotGroupSlotDim(name, dim) * (dim_var)) \
)

#define slotGroupCheckMinMax(v, _str, _min, _max) \
    _Static_assert((v) >= (_min), _str); \
    _Static_assert((v) <= (_max), _str)

#define slotGroupCheck(name) \
    slotGroupCheckMinMax(slotGroupDim(name, X), STRINGIFY(slotGroupDim(name, X)), 0, UINT8_MAX); \
    slotGroupCheckMinMax(slotGroupDim(name, Y), STRINGIFY(slotGroupDim(name, Y)), 0, UINT8_MAX); \
    slotGroupCheckMinMax(slotGroupSlotDim(name, X), STRINGIFY(slotGroupSlotDim(name, X)), 0, UINT8_MAX); \
    slotGroupCheckMinMax(slotGroupSlotDim(name, Y), STRINGIFY(slotGroupSlotDim(name, Y)), 0, UINT8_MAX); \
    slotGroupCheckMinMax(slotGroupSlotSpacing(name, X), STRINGIFY(slotGroupSlotSpacing(name, X)), 0, UINT16_MAX); \
    slotGroupCheckMinMax(slotGroupSlotSpacing(name, Y), STRINGIFY(slotGroupSlotSpacing(name, Y)), 0, UINT16_MAX); \
    slotGroupCheckMinMax(slotGroupOrigin(name, X), STRINGIFY(slotGroupOrigin(name, X)), 0, UINT16_MAX); \
    slotGroupCheckMinMax(slotGroupOrigin(name, Y), STRINGIFY(slotGroupOrigin(name, Y)), 0, UINT16_MAX); \
    _Static_assert(slotGroupIndexOffset(name) >= 0)

#define createSlotRef(slot_group, name, x, y, ref_slot_group, ref_name, ref_x, ref_y) ({ \
    const size_t index = slotGroupIndexOffset(name) + ((y) * slotGroupDim(name, X)) + (x); \
    Slot* slot = &slot_group[index]; \
    slot->index = index; \
    const size_t ref_index = slotGroupIndexOffset(ref_name) \
                            + ((ref_y) * slotGroupDim(ref_name, X)) + (ref_x); \
    slot->data.ref = &ref_slot_group[ref_index]; \
})

#define createSlot(slot_group, name, x, y) ({ \
    const size_t index = slotGroupIndexOffset(name) + ((y) * slotGroupDim(name, X)) + (x); \
    Slot* slot = &slot_group[index]; \
    slot->index = index; \
    slot->data.item = NULL; \
})

#define createSlotInline(name, x, y) ((Slot) { \
    .index = slotGroupIndexOffset(name) + ((y) * slotGroupDim(name, X)) + (x), \
    .data.item = NULL \
})

// Standard parameters
#define SLOT_WIDTH_DEFAULT 16
#define SLOT_HEIGHT_DEFAULT 16
#define SLOT_SPACING_X_DEFAULT 2
#define SLOT_SPACING_Y_DEFAULT  2
#define SLOT_DELTA_X (SLOT_WIDTH_DEFAULT + SLOT_SPACING_X_DEFAULT)
#define SLOT_DELTA_Y (SLOT_HEIGHT_DEFAULT + SLOT_SPACING_Y_DEFAULT)

Slot* slotFromScreenPosition0(const SVECTOR* screen_position,
                              Slot* group_slots,
                              const u16 group_origin_x,
                              const u16 group_origin_y,
                              const u8 group_dim_x,
                              const u8 group_dim_y,
                              const u8 slot_dim_x,
                              const u8 slot_dim_y,
                              const u8 slot_spacing_x,
                              const u8 slot_spacing_y);

// TODO: Use with input handler for Inventory and CraftingTable
#define slotFromScreenPosition(name, screen_position, group_slots) slotFromScreenPosition0( \
    screen_position, \
    group_slots, \
    slotGroupOrigin(name, X), \
    slotGroupOrigin(name, Y), \
    slotGroupDim(name, X), \
    slotGroupDim(name, Y), \
    slotGroupSlotDim(name, X), \
    slotGroupSlotDim(name, Y), \
    slotGroupSlotSpacing(name, X), \
    slotGroupSlotSpacing(name, Y), \
)

IItem* slotDirectItemGetter(Slot* slot);
void slotDirectItemSetter(Slot* slot, IItem* item);

IItem* slotRefItemGetter(Slot* slot);
void slotRefItemSetter(Slot* slot, IItem* item);

#endif // PSXMC_SLOT_H
