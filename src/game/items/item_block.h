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
#define ITEM_BLOCK_SIZE 10
#define ITEM_BLOCK_INVENTORY_SIZE 4
// TODO: Adjust this so it's within the entire slot of 16x16 (old: size = 3, scaling = 120)
#define ITEM_BLOCK_INVENTORY_SCALING 110
#define ITEM_BLOCK_INVENTORY_POSITION_RENDER_ATTRIBUTE (VECTOR) { \
    .vx = 0, \
    .vy = 0, \
    .vz = ITEM_BLOCK_INVENTORY_SCALING \
}
#define ITEM_BLOCK_INVENTORY_ROTATION_RENDER_ATTRIBUTE (SVECTOR) { \
    .vx = ONE >> 4, \
    .vy = ONE >> 3, \
    .vz = 0 \
}
#define ITEM_BLOCK_BOB_ANIM_SAMPLES 37
// Minecraft's item spin rate is 2.87675 degrees per tick
// 2.87675 / 360 = 0.0079909722
// (2.87675 / 360) * 4096 = 32.7310222222
#define ITEM_ROTATION_QUANTA 32
#define ITEM_BLOCK_BOB_ANIM_SAMPLES 37
// Minecraft's item spin rate is 2.87675 degrees per tick
// 2.87675 / 360 = 0.0079909722
// (2.87675 / 360) * 4096 = 32.7310222222
#define ITEM_ROTATION_QUANTA 32

extern const VECTOR item_stack_render_offsets[5];

#define FULL_BLOCK_FACE_INDICES_COUNT BLOCK_FACES
extern const uint8_t FULL_BLOCK_FACE_INDICES[FULL_BLOCK_FACE_INDICES_COUNT];

#define ISOMETRIC_BLOCK_FACE_INDICES_COUNT 3
extern const uint8_t ISOMETRIC_BLOCK_FACE_INDICES[ISOMETRIC_BLOCK_FACE_INDICES_COUNT];

extern const int32_t item_block_anim_sigmoid_lut[ITEM_BLOCK_BOB_ANIM_SAMPLES];
extern const int32_t item_block_anim_sin_lut[ITEM_BLOCK_BOB_ANIM_SAMPLES];

#ifndef ITEM_BLOCK_ANIM_LUT
#define ITEM_BLOCK_ANIM_LUT item_block_anim_sin_lut
#endif

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

void itemBlockApplyInventoryRenderAttributes(ItemBlock* item);

#endif // PSX_MINECRAFT_ITEM_BLOCK_H
