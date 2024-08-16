#pragma once

#ifndef PSXMC_BLOCK_AIR_H
#define PSXMC_BLOCK_AIR_H

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
    .can_harvest = toolTypeBitset(0,0,0,0,0,0), \
    .propagates_sunlight = true, \
    .propagates_blocklight = true, \
    .face_attributes = AIR_FACE_ATTRIBUTES, \
    .name = "air" \
})
#define airBlockFaceAttributes() {}

void airBlockInit(VSelf);
void AirBlock_init(VSelf);

IItem* airBlockDestroy(VSelf, bool drop_item);
IItem* AirBlock_destroy(VSelf, bool drop_item);

IItem* airBlockProvideItem(VSelf);
IItem* AirBlock_provideItem(VSelf);

impl(IBlock, AirBlock);

#endif // PSXMC_BLOCK_AIR_H
