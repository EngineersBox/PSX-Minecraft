#pragma once

#ifndef PSX_MINECRAFT_BLOCK_AIR_H
#define PSX_MINECRAFT_BLOCK_AIR_H

#include <stdbool.h>
#include <interface99.h>

#include "block.h"

DEFN_BLOCK_STATELESS(AirBlock, AIR);

DEFN_BLOCK_CONSTRUCTOR(air);

#define airBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = 0, \
    .hardness = 0, \
    .resistance = 0, \
    .type = BLOCKTYPE_EMPTY, \
    .tool_type = TOOLTYPE_NONE, \
    .tool_material = ITEMMATERIAL_NONE, \
    .name = "air" \
})

void airBlockInit(VSelf);
void AirBlock_init(VSelf);

void airBlockAccess(VSelf);
void AirBlock_access(VSelf);

IItem* airBlockDestroy(VSelf, bool drop_item);
IItem* AirBlock_destroy(VSelf, bool drop_item);

void airBlockUpdate(VSelf);
void AirBlock_update(VSelf);

IItem* airBlockProvideItem(VSelf);
IItem* AirBlock_provideItem(VSelf);

#define AirBlock_isOpaque_CUSTOM ()
impl(IBlock, AirBlock);

#endif // PSX_MINECRAFT_BLOCK_AIR_H
