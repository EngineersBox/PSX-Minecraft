#pragma once

#ifndef PSX_MINECRAFT_ITEM_H
#define PSX_MINECRAFT_ITEM_H

#include <interface99.h>
#include <stdint.h>
#include <psxgte.h>

#include "../../render/render_context.h"
#include "../../render/transforms.h"

typedef uint8_t ItemID;

typedef enum {
    ITEMTYPE_BLOCK,
    ITEMTYPE_RESOURCE,
    ITEMTYPE_TOOL
} ItemType;

typedef struct {
    ItemID id;
    ItemType type;
    uint8_t stack_size;
    uint8_t max_stack_size;
    // World position or screen position
    VECTOR position;
    VECTOR rotation;
    char* name;
} Item;

#define IItem_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, applyDamage, VSelf) \
    vfunc(void, useAction, VSelf) \
    vfunc(void, attackAction, VSelf)

interface(IItem);

IItem* itemCreate();
void itemDestroy(IItem* item);

#define DEFN_ITEM(name, ...) \
    typedef struct { \
        Item item; \
        __VA_ARGS__ \
    } name;

#endif // PSX_MINECRAFT_ITEM_H
