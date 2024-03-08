#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_H
#define PSX_MINECRAFT_ITEM_BLOCK_H

#include <stdint.h>

#include "../blocks/block.h"
#include "item.h"
#include "../../resources/texture.h"

#define ITEM_BLOCK_FACES 6

typedef struct {
    Item item;
    TextureAttributes face_attributes[ITEM_BLOCK_FACES];
} ItemBlock;

#define DEFN_ITEM_BLOCK(name, ...) \
    typedef struct { \
        ItemBlock item_block; \
        __VA_ARGS__ \
    } name;

#endif // PSX_MINECRAFT_ITEM_BLOCK_H
