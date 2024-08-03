#include "item.h"

#include <stdlib.h>
#include <stdbool.h>

#define HEIGHT_INTERVALS 2
#define RADIUS_INTERVALS 2
const u32 item_collision_intervals_height[HEIGHT_INTERVALS] = { 0, 258048, 516096 };
const u32 item_collision_intervals_radius[RADIUS_INTERVALS] = { 0, 57344 };
const PhysicsObjectConfig item_physics_object_config = (PhysicsObjectConfig) {
    .jump_height = 0, // Items can't jump
    .radius = 35840, // Width: 0.25 => Radius: ONE_BLOCK * 0.125 = 35840
    .height = 71680, // ONE_BLOCK * 0.25 = 71680
    .step_height = 0,
    .gravity = 22937, // ONE_BLOCK * 0.08 = 22937
    .collision_intervals = {
        .height_count = HEIGHT_INTERVALS,
        .radius_count = RADIUS_INTERVALS,
        .height = item_collision_intervals_height,
        .radius = item_collision_intervals_radius
    },
    .y_offset = 0,
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
        item->world_physics_object = malloc(sizeof(PhysicsObject));
        iPhysicsObjectInit(
            item->world_physics_object,
            &item_physics_object_config,
            &item_physics_object_update_handlers
        );
        item->world_entity = malloc(sizeof(Entity));
        entityInit(item->world_entity);
        return;
    }
    // In inventory
    free(item->world_physics_object);
    free(item->world_entity);
}

bool itemUpdate(Item* item,
                World* world,
                const VECTOR* player_position,
                void* ctx,
                const ItemPickupValidator validator) {
    if (item->in_world) {
        iPhysicsObjectUpdate(
            item->world_physics_object,
            world,
            ctx
        );
    }
    const int32_t sq_dist = squareDistance(player_position, &item->position);
    if (sq_dist > PICKUP_DISTANCE_SQUARED) {
        return false;
    }
    // TODO: Can we cache validator results by using the item address as an identifier?
    if (!validator(item, ctx)) {
        return false;
    }
    if (item->in_world) {
        itemSetWorldState(item, false);
        return false;
    } else if (sq_dist <= PICKUP_TO_INV_DISTANCE_SQUARED) {
        return true;
    }
    const int32_t sign_x = sign(player_position->vx - item->position.vx);
    const int32_t sign_y = sign(player_position->vy - item->position.vy);
    const int32_t sign_z = sign(player_position->vz - item->position.vz);
    item->position.vx += sign_x * PICKUP_MOVE_ANIM_DISTANCE;
    item->position.vy += sign_y * PICKUP_MOVE_ANIM_DISTANCE;
    item->position.vz += sign_z * PICKUP_MOVE_ANIM_DISTANCE;
    return false;
}

IItem* itemCreate() {
    return (IItem*) malloc(sizeof(IItem));
}

ItemActionState iitemAttackAction(VSelf) ALIAS("IItem_attackAction");
ItemActionState IItem_attackAction(VSelf) {
    return ITEM_ACTION_STATE_NONE;
}

ItemActionState iitemUseAction(VSelf) ALIAS("IItem_useAction");
ItemActionState IItem_useAction(VSelf) {
    return ITEM_ACTION_STATE_NONE;
}

void itemDestroy(IItem* item) {
    free(item);
}
