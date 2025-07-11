#include "item.h"

#include "../../core/std/stdlib.h"
#include <stdbool.h>

#include "../../logging/logging.h"
#include "items.h"
#include "../../util/memory.h"

#define HEIGHT_INTERVALS 2
#define RADIUS_INTERVALS 2
const i32 item_collision_intervals_height[HEIGHT_INTERVALS] = { 0, 71680 };
const i32 item_collision_intervals_radius[RADIUS_INTERVALS] = { 0, 35840 };
const PhysicsObjectConfig item_physics_object_config = (PhysicsObjectConfig) {
    .jump_height = 0, // Items can't jump
    .radius = 35840, // Width: 0.25 => Radius: ONE_BLOCK * (0.25 / 2) = 35840
    .height = 71680, // ONE_BLOCK * 0.25 = 71680
    .step_height = 0,
    .gravity = 22937, // ONE_BLOCK * 0.08 = 22937
    .collision_intervals = {
        .height_count = HEIGHT_INTERVALS,
        .radius_count = RADIUS_INTERVALS,
        .height = item_collision_intervals_height,
        .radius = item_collision_intervals_radius
    },
    .y_offset = 43008, // ONE_BLOCK * (0.25 * 0.6)
};
const PhysicsObjectUpdateHandlers item_physics_object_update_handlers = (PhysicsObjectUpdateHandlers) {
    .fall_handler = NULL
};
#undef HEIGHT_INTERVALS
#undef RADIUS_INTERVALS

void itemSetWorldState(Item* item, const bool in_world) {
    if (item->in_world == in_world) {
        // We are already in the requested
        // state, do nothing
        return;
    }
    item->in_world = in_world;
    if (in_world) {
        // In world
        item->world_entity = entityNew();
        entityInit(item->world_entity);
        iPhysicsObjectInit(
            &item->world_entity->physics_object,
            &item_physics_object_config,
            &item_physics_object_update_handlers
        );
        return;
    }
    // In inventory
    free(item->world_entity);
    item->position = vec3_i32(0);
}

bool itemUpdate(Item* item,
                World* world,
                const VECTOR* player_position,
                void* ctx,
                const ItemPickupValidator validator) {
    if (item->in_world) {
        iPhysicsObjectUpdate(
            &item->world_entity->physics_object,
            world,
            ctx
        );
    }
    const VECTOR item_pos = vec3_add(
        vec3_i32(
            (item->world_entity->physics_object.position.vx) >> FIXED_POINT_SHIFT,
            (-item->world_entity->physics_object.position.vy) >> FIXED_POINT_SHIFT,
            (item->world_entity->physics_object.position.vz) >> FIXED_POINT_SHIFT
        ),
        item->position
    );
    const i32 sq_dist = squareDistance(player_position, &item_pos);
    if (sq_dist > PICKUP_DISTANCE_SQUARED) {
        return false;
    }
    // TODO: Can we cache validator results by using the item address as an identifier?
    if (!validator(item, ctx)) {
        return false;
    }
    if (sq_dist <= PICKUP_TO_INV_DISTANCE_SQUARED) {
        if (item->in_world) {
            itemSetWorldState(item, false);
        }
        return true;
    }
    const i32 sign_x = sign(player_position->vx - item_pos.vx);
    const i32 sign_y = sign(player_position->vy - item_pos.vy);
    const i32 sign_z = sign(player_position->vz - item_pos.vz);
    item->position.vx += sign_x * PICKUP_MOVE_ANIM_DISTANCE;
    item->position.vy += sign_y * PICKUP_MOVE_ANIM_DISTANCE;
    item->position.vz += sign_z * PICKUP_MOVE_ANIM_DISTANCE;
    return false;
}

IItem* itemCreate() {
    IItem* iitem = malloc(sizeof(IItem));
    zeroed(iitem);
    return iitem;
}

void iitemApplyDamage(VSelf, i16 damage) ALIAS("IItem_applyDamage");
void IItem_applyDamage(VSelf, i16 damage) {
    VSELF(Item);
    if (!itemHasDurability(self->id)) {
        return;
    }
    const i16 new_durability = ((i16) self->durability) - damage;
    self->durability = (u8) clamp(
        new_durability,
        0,
        itemGetMaxDurability(self->id)
    );
}

ItemActionState iitemAttackAction(VSelf) ALIAS("IItem_attackAction");
ItemActionState IItem_attackAction(UNUSED VSelf) {
    return ITEM_ACTION_STATE_NONE;
}

ItemActionState iitemUseAction(VSelf) ALIAS("IItem_useAction");
ItemActionState IItem_useAction(UNUSED VSelf) {
    return ITEM_ACTION_STATE_NONE;
}

void itemDestroy(IItem* item) {
    free(item);
}
