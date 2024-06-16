#pragma once

#ifndef PSX_MINECRAFT_ITEM_H
#define PSX_MINECRAFT_ITEM_H

#include <interface99.h>
#include <stdint.h>
#include <psxgte.h>
#include <stdbool.h>

#include "../../render/renderable.h"
#include "../../math/math_utils.h"
#include "../../util/preprocessor.h"

#define PICKUP_DISTANCE 154
#define PICKUP_DISTANCE_SQUARED pow2(PICKUP_DISTANCE)
#define PICKUP_TO_INV_DISTANCE 25
#define PICKUP_TO_INV_DISTANCE_SQUARED pow2(PICKUP_TO_INV_DISTANCE)
#define PICKUP_MOVE_ANIM_DISTANCE 8

typedef u8 ItemID;

#define ITEM_TYPE_COUNT 4
#define ITEM_TYPE_COUNT_BITS 2
typedef enum {
    ITEMTYPE_BLOCK,
    ITEMTYPE_RESOURCE,
    ITEMTYPE_TOOL,
    ITEMTYPE_ARMOUR
} ItemType;

#define TOOL_TYPE_COUNT 6
#define TOOL_TYPE_COUNT_BITS 3
typedef enum ToolType {
    TOOLTYPE_NONE = 0,
    TOOLTYPE_PICKAXE,
    TOOLTYPE_AXE,
    TOOLTYPE_SWORD,
    TOOLTYPE_SHOVEL,
    TOOLTYPE_HOE
} ToolType;

#define TOOL_MATERIAL_COUNT 6
#define TOOL_MATERIAL_COUNT_BITS 3
typedef enum ToolMaterial {
    TOOLMATERIAL_NONE = 0,
    TOOLMATERIAL_WOOD,
    TOOLMATERIAL_STONE,
    TOOLMATERIAL_IRON,
    TOOLMATERIAL_GOLD,
    TOOLMATERIAL_DIAMOND
} ToolMaterial;

typedef struct ItemAttributes {
    ItemType type: ITEM_TYPE_COUNT_BITS;
    ToolType tool_type: TOOL_TYPE_COUNT_BITS;
    ToolMaterial tool_material: TOOL_MATERIAL_COUNT;
} ItemAttributes;

typedef struct Item {
    ItemID id;
    u8 metadata_id;
    uint8_t stack_size;
    uint8_t max_stack_size;
    uint8_t bob_offset;
    uint8_t bob_direction;
    bool picked_up;
    // World position or screen position
    VECTOR position;
    SVECTOR rotation;
    char* name;
} Item;

/**
 * @brief Determine if an item can be picked up
 * @param item Item to check against
 * @return true if item can be picked up, false otherwise
 */
typedef bool (*ItemPickupValidator)(const Item* item);

bool itemUpdate(Item* item, const VECTOR* player_position, const ItemPickupValidator validator);

#define IItem_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, applyDamage, VSelf) \
    vfunc(void, useAction, VSelf) \
    vfunc(void, attackAction, VSelf) \
    vfunc(void, destroy, VSelf)

// Superinterface: renderWorld, renderInventory, renderHand
#define IItem_EXTENDS (Renderable)
interface(IItem);

void itemDestroy(IItem* item);
ALLOC_CALL(itemDestroy, 1) IItem* itemCreate();

#define DEFN_ITEM(name, ...) \
    typedef struct { \
        Item item; \
        __VA_ARGS__ \
    } name;

#endif // PSX_MINECRAFT_ITEM_H
