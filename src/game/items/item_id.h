#pragma once

#ifndef PSX_MINECRAFT_ITEM_ID_H
#define PSX_MINECRAFT_ITEM_ID_H

#include "../../util/preprocessor.h"

#define MK_ITEM_LIST(f) \
    f(ITEMID_AIR) \
    f(ITEMID_STONE) \
    f(ITEMID_DIRT) \
    f(ITEMID_GRASS)

typedef enum {
    MK_ITEM_LIST(P99_ENUM_ENTRY)
} EItemID;

extern const char* EITEMID_NAMES[];

#define itemIdStringify(id) EITEMID_NAMES[(id)]

#endif // PSX_MINECRAFT_ITEM_ID_H
