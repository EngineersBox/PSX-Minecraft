#pragma once

#ifndef PSX_MINECRAFT_ITEM_BLOCK_H
#define PSX_MINECRAFT_ITEM_BLOCK_H

#include <stdint.h>

#include "../blocks/block.h"
#include "item.h"
#include "../../resources/texture.h"
#include "../../render/render_context.h"
#include "../../render/transforms.h"

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

#define itemBlockReplicateFaceAttributes(_item_block_to, _block_from) \
    (_item_block_to).face_attributes[0] = (_block_from).face_attributes[0]; \
    (_item_block_to).face_attributes[1] = (_block_from).face_attributes[1]; \
    (_item_block_to).face_attributes[2] = (_block_from).face_attributes[2]; \
    (_item_block_to).face_attributes[3] = (_block_from).face_attributes[3]; \
    (_item_block_to).face_attributes[4] = (_block_from).face_attributes[4]; \
    (_item_block_to).face_attributes[5] = (_block_from).face_attributes[5];

void itemBlockRenderWorld(ItemBlock* item, RenderContext* ctx, Transforms* transforms);
void itemBlockRenderInventory(ItemBlock* item, RenderContext* ctx, Transforms* transforms);
void itemBlockRenderHand(ItemBlock* item, RenderContext* ctx, Transforms* transforms);

#endif // PSX_MINECRAFT_ITEM_BLOCK_H
