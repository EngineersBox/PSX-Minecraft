#pragma once

#ifndef PSX_MINECRAFT_ITEM_H
#define PSX_MINECRAFT_ITEM_H

#include <interface99.h>
#include <stdint.h>
#include <psxgte.h>
#include <stdbool.h>

#include "../../render/renderable.h"
#include "../../math/math_utils.h"

#define PICKUP_DISTANCE 154
#define PICKUP_DISTANCE_SQUARED pow2(PICKUP_DISTANCE)
#define PICKUP_TO_INV_DISTANCE 25
#define PICKUP_TO_INV_DISTANCE_SQUARED pow2(PICKUP_TO_INV_DISTANCE)
#define PICKUP_MOVE_ANIM_DISTANCE 8

typedef uint8_t ItemID;

typedef enum {
    ITEMTYPE_BLOCK,
    ITEMTYPE_RESOURCE,
    ITEMTYPE_TOOL
} ItemType;

typedef struct {
    ItemID id;
    ItemType type;
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

#define IItem_EXTENDS (Renderable)
interface(IItem);

extern const IItem IITEM_NULL;

IItem* itemCreate();
void itemDestroy(IItem* item);

#define DEFN_ITEM(name, ...) \
    typedef struct { \
        Item item; \
        __VA_ARGS__ \
    } name;

#endif // PSX_MINECRAFT_ITEM_H
