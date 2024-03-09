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

#define itemBlockReplicateFaceAttributes(_item_block, _block) \
    (_item_block).face_attributes[0] = (_block).face_attributes[0]; \
    (_item_block).face_attributes[1] = (_block).face_attributes[1]; \
    (_item_block).face_attributes[2] = (_block).face_attributes[2]; \
    (_item_block).face_attributes[3] = (_block).face_attributes[3]; \
    (_item_block).face_attributes[4] = (_block).face_attributes[4]; \
    (_item_block).face_attributes[5] = (_block).face_attributes[5];

#endif // PSX_MINECRAFT_ITEM_BLOCK_H
