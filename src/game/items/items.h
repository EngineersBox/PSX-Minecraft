#pragma once

#ifndef _PSX_MINECRAFT__GAME_ITEMS__ITEMS_H_
#define _PSX_MINECRAFT__GAME_ITEMS__ITEMS_H_

#include "item.h"
// These includes allow for only including items.h
// and getting access to all items instantly
#include "blocks/block_stone.h"
#include "blocks/block_grass.h"
#include "blocks/block_dirt.h"
#include "blocks/block_cobblestone.h"

#define ITEM_COUNT 6

extern ItemAttributes item_attributes[ITEM_COUNT];

#define itemGetAttribute(id, attr) (item_attributes[(id)].attr)
#define itemGetName(id) blockGetAttribute(id, name)
#define itemGetToolType(id) itemGetAttribute(id, tool_type)
#define itemGetToolMaterial(id) itemGetAttribute(id, tool_material)

#endif // _PSX_MINECRAFT__GAME_ITEMS__ITEMS_H_