#pragma once

#ifndef PSX_MINECRAFT_ITEM_H
#define PSX_MINECRAFT_ITEM_H

#include <interface99.h>
#include <stdint.h>

typedef uint8_t ItemID;

typedef struct {
    ItemID id;
    uint8_t stack_size;
    uint8_t max_stack_size;
    char* name;
} Item;

#define ItemRenderer_IFACE \
    vfunc(void, render, VSelf, Item* item)

interface(ItemRenderer);

#define IItem_IFACE \
    vfunc(void, useAction, VSelf) \
    vfunc(void, attackAction, VSelf)

#define IItem_EXTENDS (ItemRenderer)

interface(IItem);

#define DEFN_ITEM(name, ...) \
    typedef struct { \
        Item item; \
        __VA_ARGS__ \
    } name;

#endif // PSX_MINECRAFT_ITEM_H
