#pragma once

#ifndef PSXMC_SLOT_H
#define PSXMC_SLOT_H

#include <stdbool.h>
#include <psxgte.h>
#include <assert.h>
#include <stdint.h>

#include "../items/item.h"

typedef struct Slot {
    union {
        // Actual data
        IItem* item;
        // Reference to slot elsewhere, i.e. inventory
        // hotbar slots mapping to actual hotbar slots
        struct Slot* ref;
    } data;
    // Screen space position
    DVECTOR position;
    DVECTOR dimensions;
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

// Utility accessors for slot group definitions
#define slotGroupDim(name, dim) name##_SLOT_GROUP_DIMENSIONS_##dim
#define slotGroupSlotDim(name, dim) name##_SLOT_GROUP_SLOT_DIMENSIONS_##dim
#define slotGroupSlotSpacing(name, dim) name##_SLOT_GROUP_SLOT_SPACING_##dim
#define slotGroupOrigin(name, dim) name##_SLOT_GROUP_ORIGIN_##dim

#define xstr(s) str(s)
#define str(s) #s
#define checkMinMax(v, _str, _min, _max) \
    _Static_assert((v) >= (_min), _str); \
    _Static_assert((v) <= (_max), _str)

#define slotGroupCheck(name) \
    checkMinMax(slotGroupDim(name, X), str(slotGroupDim(name, X)), 0, UINT8_MAX); \
    checkMinMax(slotGroupDim(name, Y), str(slotGroupDim(name, Y)), 0, UINT8_MAX); \
    checkMinMax(slotGroupSlotDim(name, X), str(slotGroupSlotDim(name, X)), 0, UINT8_MAX); \
    checkMinMax(slotGroupSlotDim(name, Y), str(slotGroupSlotDim(name, Y)), 0, UINT8_MAX); \
    checkMinMax(slotGroupSlotSpacing(name, X), str(slotGroupSlotSpacing(name, X)), 0, UINT16_MAX); \
    checkMinMax(slotGroupSlotSpacing(name, Y), str(slotGroupSlotSpacing(name, Y)), 0, UINT16_MAX); \
    checkMinMax(slotGroupOrigin(name, X), str(slotGroupOrigin(name, X)), 0, UINT16_MAX); \
    checkMinMax(slotGroupOrigin(name, Y), str(slotGroupOrigin(name, Y)), 0, UINT16_MAX)

#ifndef INV_SLOT_WIDTH
#define INV_SLOT_WIDTH 16
#endif
#ifndef INV_SLOT_HEIGHT
#define INV_SLOT_HEIGHT 16
#endif
#ifndef INV_SLOT_DIMS
#define INV_SLOT_DIMS (DVECTOR) { \
    .vx = INV_SLOT_WIDTH, \
    .vy = INV_SLOT_HEIGHT \
}
#endif

#ifndef INV_SLOT_POS_DELTA
#define INV_SLOT_POS_DELTA 18
#endif

// Player Inventory [START]
// TODO: Figure out these values

// = Player Inventory - Armor [START]
// +-+
// | |
// +-+
// | |
// +-+
// | |
// +-+
// | |
// +-+

#ifndef PLAYER_INV_ARMOR_HELMET_POS_X
#define PLAYER_INV_ARMOR_HELMET_POS_X 80
#endif
#ifndef PLAYER_INV_ARMOR_HELMET_POS_Y
#define PLAYER_INV_ARMOR_HELMET_POS_Y 45
#endif
#define PLAYER_INV_ARMOR_HELMET_POS (DVECTOR) { \
    .vx = PLAYER_INV_ARMOR_HELMET_POS_X, \
    .vy = PLAYER_INV_ARMOR_HELMET_POS_Y \
}

#ifndef PLAYER_INV_ARMOR_CHESTPLATE_POS_X
#define PLAYER_INV_ARMOR_CHESTPLATE_POS_X PLAYER_INV_ARMOR_HELMET_POS_X
#endif
#ifndef PLAYER_INV_ARMOR_CHESTPLATE_POS_Y
#define PLAYER_INV_ARMOR_CHESTPLATE_POS_Y (PLAYER_INV_ARMOR_HELMET_POS_Y + INV_SLOT_POS_DELTA)
#endif
#define PLAYER_INV_ARMOR_CHESTPLATE_POS (DVECTOR) { \
    .vx = PLAYER_INV_ARMOR_CHESTPLATE_POS_X, \
    .vy = PLAYER_INV_ARMOR_CHESTPLATE_POS_Y \
}

#ifndef PLAYER_INV_ARMOR_LEGGINGS_POS_X
#define PLAYER_INV_ARMOR_LEGGINGS_POS_X PLAYER_INV_ARMOR_CHESTPLATE_POS_X
#endif
#ifndef PLAYER_INV_ARMOR_LEGGINGS_POS_Y
#define PLAYER_INV_ARMOR_LEGGINGS_POS_Y (PLAYER_INV_ARMOR_CHESTPLATE_POS_Y + INV_SLOT_POS_DELTA)
#endif
#define PLAYER_INV_ARMOR_LEGGINGS_POS (DVECTOR) { \
    .vx = PLAYER_INV_ARMOR_LEGGINGS_POS_X, \
    .vy = PLAYER_INV_ARMOR_LEGGINGS_POS_Y \
}

#ifndef PLAYER_INV_ARMOR_BOOTS_POS_X
#define PLAYER_INV_ARMOR_BOOTS_POS_X PLAYER_INV_ARMOR_LEGGINGS_POS_X
#endif
#ifndef PLAYER_INV_ARMOR_BOOTS_POS_Y
#define PLAYER_INV_ARMOR_BOOTS_POS_Y (PLAYER_INV_ARMOR_LEGGINGS_POS_Y + INV_SLOT_POS_DELTA)
#endif
#define PLAYER_INV_ARMOR_BOOTS_POS (DVECTOR) { \
    .vx = PLAYER_INV_ARMOR_BOOTS_POS_X, \
    .vy = PLAYER_INV_ARMOR_BOOTS_POS_Y \
}

// = Player Inventory - Armor [END]

// = Player Inventory - Crafting [START]
// +-+-+
// | | |   +-+
// +-+-+   | |
// | | |   +-+
// +-+-+

#ifndef PLAYER_INV_CRAFTING_TOP_LEFT_POS_X
#define PLAYER_INV_CRAFTING_TOP_LEFT_POS_X 160
#endif
#ifndef PLAYER_INV_CRAFTING_TOP_LEFT_POS_Y
#define PLAYER_INV_CRAFTING_TOP_LEFT_POS_Y 63
#endif
#define PLAYER_INV_CRAFTING_TOP_LEFT_POS (DVECTOR) { \
    .vx = PLAYER_INV_CRAFTING_TOP_LEFT_POS_X, \
    .vy = PLAYER_INV_CRAFTING_TOP_LEFT_POS_Y \
}

#ifndef PLAYER_INV_CRAFTING_TOP_RIGHT_POS_X
#define PLAYER_INV_CRAFTING_TOP_RIGHT_POS_X (PLAYER_INV_CRAFTING_TOP_LEFT_POS_X + INV_SLOT_POS_DELTA)
#endif
#ifndef PLAYER_INV_CRAFTING_TOP_RIGHT_POS_Y
#define PLAYER_INV_CRAFTING_TOP_RIGHT_POS_Y PLAYER_INV_CRAFTING_TOP_LEFT_POS_Y
#endif
#define PLAYER_INV_CRAFTING_TOP_RIGHT_POS (DVECTOR) { \
    .vx = PLAYER_INV_CRAFTING_TOP_RIGHT_POS_X, \
    .vy = PLAYER_INV_CRAFTING_TOP_RIGHT_POS_Y \
}

#ifndef PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS_X
#define PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS_X PLAYER_INV_CRAFTING_TOP_LEFT_POS_X
#endif
#ifndef PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS_Y
#define PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS_Y (PLAYER_INV_CRAFTING_TOP_LEFT_POS_Y + INV_SLOT_POS_DELTA)
#endif
#define PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS (DVECTOR) { \
    .vx = PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS_X, \
    .vy = PLAYER_INV_CRAFTING_BOTTOM_LEFT_POS_Y \
}

#ifndef PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS_X
#define PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS_X (PLAYER_INV_CRAFTING_TOP_LEFT_POS_X + INV_SLOT_POS_DELTA)
#endif
#ifndef PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS_Y
#define PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS_Y (PLAYER_INV_CRAFTING_TOP_LEFT_POS_Y + INV_SLOT_POS_DELTA)
#endif
#define PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS (DVECTOR) { \
    .vx = PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS_X, \
    .vy = PLAYER_INV_CRAFTING_BOTTOM_RIGHT_POS_Y \
}

#ifndef PLAYER_INV_CRAFTING_RESULT_POS_X
#define PLAYER_INV_CRAFTING_RESULT_POS_X 216
#endif
#ifndef PLAYER_INV_CRAFTING_RESULT_POS_Y
#define PLAYER_INV_CRAFTING_RESULT_POS_Y 73
#endif
#define PLAYER_INV_CRAFTING_RESULT_POS (DVECTOR) { \
    .vx = PLAYER_INV_CRAFTING_RESULT_POS_X, \
    .vy = PLAYER_INV_CRAFTING_RESULT_POS_Y \
}

// = Player Inventory - Crafing [END]

// = Player Inventory - Storage [START]

#ifndef PLAYER_INV_STORAGE_BASE_POS_X
#define PLAYER_INV_STORAGE_BASE_POS_X 80
#endif
#ifndef PLAYER_INV_STORAGE_BASE_POS_Y
#define PLAYER_INV_STORAGE_BASE_POS_Y 121
#endif
#ifndef PLAYER_INV_STORAGE_SLOTS_WIDTH
#define PLAYER_INV_STORAGE_SLOTS_WIDTH 9
#endif
#ifndef PLAYER_INV_STORAGE_SLOTS_HEIGHT
#define PLAYER_INV_STORAGE_SLOTS_HEIGHT 3
#endif
#define playerInvStoragePos(x, y) (DVECTOR) { \
    .vx = PLAYER_INV_STORAGE_BASE_POS_X + ((x) * INV_SLOT_POS_DELTA), \
    .vy = PLAYER_INV_STORAGE_BASE_POS_Y + ((y) * INV_SLOT_POS_DELTA) \
}

// = Player Inventory - Storage [END]

// = Player Inventory - Hotbar [START]

#ifndef PLAYER_INV_HOTBAR_BASE_POS_X
#define PLAYER_INV_HOTBAR_BASE_POS_X 80
#endif
#ifndef PLAYER_INV_HOTBAR_BASE_POS_Y
#define PLAYER_INV_HOTBAR_BASE_POS_Y 179
#endif
#define playerInvHotbarPos(x) (DVECTOR) { \
    .vx = PLAYER_INV_HOTBAR_BASE_POS_X + ((x) * INV_SLOT_POS_DELTA), \
    .vy = PLAYER_INV_HOTBAR_BASE_POS_Y \
}

// = Player Inventory - Hotbar [END]

// Player Inventory [END]

// Hotbar [START]

#ifndef HOTBAR_BASE_POS_X
#define HOTBAR_BASE_POS_X 72
#endif
#ifndef HOTBAR_BASE_POS_Y
#define HOTBAR_BASE_POS_Y 220
#endif
#ifndef HOTBAR_SLOT_POS_DELTA
#define HOTBAR_SLOT_POS_DELTA 20
#endif
#ifndef HOTBAR_SLOT_WIDTH
#define HOTBAR_SLOT_WIDTH 15
#endif
#ifndef HOTBAR_SLOT_HEIGHT
#define HOTBAR_SLOT_HEIGHT 15
#endif
#ifndef HOTBAR_SLOT_DIMS
#define HOTBAR_SLOT_DIMS (DVECTOR) { \
    .vx = HOTBAR_SLOT_WIDTH, \
    .vy = HOTBAR_SLOT_HEIGHT \
}
#endif
#ifndef HOTBAR_SLOT_BLOCK_OFFSET_X
#define HOTBAR_SLOT_BLOCK_OFFSET_X 8
#endif
#ifndef HOTBAR_SLOT_BLOCK_OFFSET_Y
#define HOTBAR_SLOT_BLOCK_OFFSET_Y 7
#endif
#define hotbarSlotPos(x, y) (DVECTOR) { \
    .vx = HOTBAR_BASE_POS_X + ((x) * HOTBAR_SLOT_POS_DELTA), \
    .vy = HOTBAR_BASE_POS_Y + ((y) * HOTBAR_SLOT_POS_DELTA) \
}

// Hotbar [END]

#endif // PSXMC_SLOT_H
