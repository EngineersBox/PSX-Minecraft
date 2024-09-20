#pragma once

#ifndef _PSXMC__GAME_ITEMS__ITEMS_H_
#define _PSXMC__GAME_ITEMS__ITEMS_H_

#include "item.h"
// These includes allow for only including items.h
// and getting access to all items instantly
#include "blocks/item_block_stone.h"
#include "blocks/item_block_grass.h"
#include "blocks/item_block_dirt.h"
#include "blocks/item_block_cobblestone.h"

#define ITEM_COUNT 6

extern ItemConstructor item_constructors[ITEM_COUNT];
extern ItemAttributes item_attributes[ITEM_COUNT];

#define itemGetAttribute(id, attr) (item_attributes[(id)].attr)
#define itemGetName(id) itemGetAttribute(id, name)
#define itemGetType(id) itemGetAttribute(id, type)
#define itemGetMaxStackSize(id) itemGetAttribute(id, max_stack_size)
#define itemGetToolType(id) itemGetAttribute(id, tool_type)
#define itemGetArmourType(id) itemGetAttribute(id, armour_type)
#define itemGetMaterial(id) itemGetAttribute(id, material)
#define itemHasDurability(id) itemGetAttribute(id, has_durability)

void itemsInitialiseBuiltin();

#endif // _PSXMC__GAME_ITEMS__ITEMS_H_
