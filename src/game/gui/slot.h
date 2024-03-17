#pragma once

#ifndef PSX_MINECRAFT_SLOT_H
#define PSX_MINECRAFT_SLOT_H

#include <stdbool.h>
#include <psxgte.h>

#include "../items/item.h"

typedef struct {
    IItem* item;
    DVECTOR position;
    DVECTOR dimensions;
    bool blocked;
} Slot;

#endif // PSX_MINECRAFT_SLOT_H
