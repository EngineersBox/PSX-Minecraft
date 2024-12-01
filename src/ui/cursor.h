#pragma once

#ifndef _PSXMC__UI__CURSOR_H_
#define _PSXMC__UI__CURSOR_H_

#include <psxgte.h>

#include "../game/items/item.h"

typedef struct Cursor {
    IItem* held_item;
    SVECTOR screen_position;
} Cursor;

#endif // _PSXMC__UI__CURSOR_H_
