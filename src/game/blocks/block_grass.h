#pragma once

#ifndef PSX_MINECRAFT_BLOCK_GRASS_H
#define PSX_MINECRAFT_BLOCK_GRASS_H

#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(GRASS, GrassBlock);

IBlock* grassBlockCreate();

// ONE * 0.6 = 2457
#define grassBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = 2457, \
    .resistance = 12288, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_SHOVEL, \
    .tool_material = TOOLMATERIAL_WOOD, \
    .name = "grass" \
})

void grassBlockInit(VSelf);
void GrassBlock_init(VSelf);

void grassBlockAccess(VSelf);
void GrassBlock_access(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* grassBlockDestroy(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* GrassBlock_destroy(VSelf);

void grassBlockUpdate(VSelf);
void GrassBlock_update(VSelf);

// TODO: REVERT THIS isOpaque IMPLEMENTATION WHEN DONE TESTING GLASS-LIKE TRANSPARENCY IN CHUNK MESH
// #define GrassBlock_isOpaque_CUSTOM ()
bool grassBlockIsOpaque(VSelf, FaceDirection face_dir);
bool GrassBlock_isOpaque(VSelf, FaceDirection face_dir);

// #define GrassBlock_opaqueBitset_CUSTOM ()
u8 grassBlockOpaqueBitset(VSelf);
u8 GrassBlock_opaqueBitset(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* grassBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* GrassBlock_provideItem(VSelf);

impl(IBlock, GrassBlock);

#endif // PSX_MINECRAFT_BLOCK_GRASS_H
