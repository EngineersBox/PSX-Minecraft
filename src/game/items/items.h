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
#include "blocks/item_block_crafting_table.h"

#define ITEM_COUNT 59
#define DEBUG_ITEM_ID_CHECK 1

extern ItemConstructor item_constructors[ITEM_COUNT];
extern ItemAttributes item_attributes[ITEM_COUNT];

#if defined(DEBUG_ITEM_ID_CHECK) && DEBUG_ITEM_ID_CHECK == 1
#include "../../logging/logging.h"
#define itemGetAttribute(id, attr) ({ \
    __typeof__(id) _id = (id); \
    if (_id >= ITEM_COUNT) { \
        errorAbort("[ERROR] Invalid item id: %d\n", id); \
    } \
    item_attributes[_id].attr; \
})
#else
#define itemGetAttribute(id, attr) (item_attributes[(id)].attr)
#endif
#define itemGetName(id) itemGetAttribute(id, name)
#define itemGetType(id) itemGetAttribute(id, type)
#define itemGetMaxStackSize(id) itemGetAttribute(id, max_stack_size)
#define itemGetToolType(id) itemGetAttribute(id, tool_type)
#define itemGetArmourType(id) itemGetAttribute(id, armour_type)
#define itemGetMaterial(id) itemGetAttribute(id, material)
#define itemHasDurability(id) itemGetAttribute(id, has_durability)

void itemsInitialiseBuiltin();

#endif // _PSXMC__GAME_ITEMS__ITEMS_H_
