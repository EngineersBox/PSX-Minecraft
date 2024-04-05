#include "item.h"

#include <stdlib.h>
#include <stdbool.h>

bool itemUpdate(Item* item, const VECTOR* player_position, const ItemPickupValidator validator) {
    const int32_t sq_dist = squareDistance(player_position, &item->position);
    if (sq_dist > PICKUP_DISTANCE_SQUARED) {
        return false;
    }
    // TODO: Can we cache validator results by using the item address as an identifier?
    if (!validator(item)) {
        return false;
    }
    if (!item->picked_up) {
        item->picked_up = true;
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

void itemDestroy(IItem* item) {
    free(item);
}