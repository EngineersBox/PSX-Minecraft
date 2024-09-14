#pragma once

#ifndef _PSXMC__GAME_BLOCKS__BLOCK_TEST_H_
#define _PSXMC__GAME_BLOCKS__BLOCK_TEST_H_

#include <interface99.h>

#include "block.h"
DEFN_BLOCK_STATELESS(TestBlock, TEST);

DEFN_BLOCK_CONSTRUCTOR(test);

#define testBlockCreateAttributes() ((BlockAttributes) { \
    .slipperiness = BLOCK_DEFAULT_SLIPPERINESS, \
    .hardness = BLOCK_DEFAULT_HARDNESS, \
    .resistance = BLOCK_DEFAULT_RESISTANCE, \
    .type = BLOCKTYPE_SOLID, \
    .tool_type = TOOLTYPE_NONE, \
    .tool_material = ITEMMATERIAL_NONE, \
    .can_harvest = toolTypeBitset(1,1,1,1,1,1), \
    .propagates_sunlight =  false , \
    .propagates_blocklight =  false , \
    .face_attributes TEST_FACE_ATTRIBUTES, \
    .name = "test" \
})

#define testBlockFaceAttributes() P99_PROTECT({\
defaultFaceAttributes(12), \
defaultTintedFaceAttributes(1,2,3,4,5,6) \
})

void testBlockInit(VSelf);
void TestBlock_init(VSelf);

ALLOC_CALL(itemDestroy, 1) IItem* testBlockDestroy(VSelf, bool drop_item);
ALLOC_CALL(itemDestroy, 1) IItem* TestBlock_destroy(VSelf, bool drop_item);

ALLOC_CALL(itemDestroy, 1) IItem* testBlockProvideItem(VSelf);
ALLOC_CALL(itemDestroy, 1) IItem* TestBlock_provideItem(VSelf);
#define TestBlock_canPlace_CUSTOM ()
bool testBlockCanPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb);
bool TestBlock_canPlace(VSelf, const World* world, const VECTOR* position, const AABB* player_aabb);
impl(IBlock, TestBlock);

#endif // _PSXMC__GAME_BLOCKS__BLOCK_TEST_H_