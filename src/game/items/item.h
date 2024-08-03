#pragma once

#include "psxapi.h"
#ifndef PSXMC_ITEM_H
#define PSXMC_ITEM_H

#include <interface99.h>
#include <stdint.h>
#include <psxgte.h>
#include <stdbool.h>

#include "../../render/renderable.h"
#include "../../math/math_utils.h"
#include "../../math/vector.h"
#include "../../util/preprocessor.h"
#include "tools/tool_type.h"
#include "../../entity/entity.h"
#include "../../physics/physics_object.h"

#define PICKUP_DISTANCE 154
#define PICKUP_DISTANCE_SQUARED (PICKUP_DISTANCE * PICKUP_DISTANCE)
#define PICKUP_TO_INV_DISTANCE 25
#define PICKUP_TO_INV_DISTANCE_SQUARED (PICKUP_TO_INV_DISTANCE * PICKUP_TO_INV_DISTANCE)
#define PICKUP_MOVE_ANIM_DISTANCE 8

typedef u8 ItemID;

#define ITEM_TYPE_COUNT 4
#define ITEM_TYPE_COUNT_BITS 2
typedef enum ItemType {
    ITEMTYPE_BLOCK,
    ITEMTYPE_RESOURCE,
    ITEMTYPE_TOOL,
    ITEMTYPE_ARMOUR
} ItemType;

#define ARMOUR_TYPE_COUNT 5
#define ARMOUR_TYPE_COUNT_BITS 3
typedef enum ArmourType {
    ARMOURTYPE_NONE = 0,
    ARMOURTYPE_HELMET,
    ARMOURTYPE_CHESTPLATE,
    ARMOURTYPE_LEGGINGS,
    ARMOURTYPE_BOOTS
} ArmourType;

#define ITEM_MATERIAL_COUNT 6
#define ITEM_MATERIAL_COUNT_BITS 3
typedef enum ItemMaterial {
    ITEMMATERIAL_NONE = 0,
    ITEMMATERIAL_WOOD,
    ITEMMATERIAL_STONE,
    ITEMMATERIAL_IRON,
    ITEMMATERIAL_GOLD,
    ITEMMATERIAL_DIAMOND
} ItemMaterial;

typedef struct ItemAttributes {
    u8 max_stack_size;
    ItemType type: ITEM_TYPE_COUNT_BITS;
    ToolType tool_type: TOOL_TYPE_COUNT_BITS;
    ArmourType armour_type: ARMOUR_TYPE_COUNT_BITS;
    ItemMaterial material: ITEM_MATERIAL_COUNT_BITS;
    bool has_durability: 1;
    u16 _pad: 3;
    char* name;
} ItemAttributes;

#define ITEM_ACTION_STATE_COUNT 3
#define ITEM_ACTION_STATE_COUNT_BITS 2
typedef enum ItemActionState {
    ITEM_ACTION_STATE_NONE = 0,
    ITEM_ACTION_STATE_DESTROY,
    ITEM_ACTION_STATE_USED
} ItemActionState;

extern const u32 item_collision_intervals_height[];
extern const u32 item_collision_intervals_radius[];
extern const PhysicsObjectConfig item_physics_object_config;
extern const PhysicsObjectUpdateHandlers item_physics_object_update_handlers;

typedef struct Item {
    ItemID id;
    u8 metadata_id;
    u32 durability;
    u8 stack_size;
    u8 bob_offset;
    u8 bob_direction;
    bool in_world;
    // World position or screen position
    // conditional on in_world being false
    VECTOR position;
    SVECTOR rotation;
    // Only present (non-NULL) when the
    // in_world flag is true
    PhysicsObject* world_physics_object;
    Entity* world_entity;
} Item;

FWD_DECL typedef struct World World;

/**
 * @brief Determine if an item can be picked up
 * @param item Item to check against
 * @param ctx Context object
 * @return true if item can be picked up, false otherwise
 */
typedef bool (*ItemPickupValidator)(const Item* item, void* ctx);

bool itemUpdate(Item* item,
                World* world,
                const VECTOR* player_position,
                void* ctx,
                const ItemPickupValidator validator);
void itemSetWorldState(Item* item, const bool in_world);

#define IItem_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, applyDamage, VSelf) \
    vfuncDefault(ItemActionState, useAction, VSelf) \
    vfuncDefault(ItemActionState, attackAction, VSelf) \
    vfunc(void, destroy, VSelf)

ItemActionState iitemAttackAction(VSelf);
ItemActionState IItem_attackAction(VSelf);

ItemActionState iitemUseAction(VSelf);
ItemActionState IItem_useAction(VSelf);

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

#define declareItem(_id, _metadata_id, _durability, _stack_size, _in_world, _position, _rotation) ((Item) { \
    .id = (_id), \
    .metadata_id = (_metadata_id), \
    .durability = (_durability), \
    .stack_size = (_stack_size), \
    .bob_offset = 0, \
    .bob_direction = 0, \
    .in_world = (_in_world), \
    .position = (_position), \
    .rotation = (_rotation) \
})
#define declareSimpleItemMeta(_id, _metadata_id, _durability) declareItem( \
    _id, \
    _metadata_id, \
    _durability, \
    0, \
    false, \
    vec3_i32_all(0), \
    vec3_i16_all(0) \
)
#define declareSimpleItem(_id, _durability) declareSimpleItemMeta(_id, 0, _durability)

#endif // PSXMC_ITEM_H
