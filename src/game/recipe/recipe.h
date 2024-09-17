#pragma once

#ifndef _PSXMC__GAME_RECIPE__RECIPE_H_
#define _PSXMC__GAME_RECIPE__RECIPE_H_

#include <stdbool.h>

#include "../items/item_id.h"
#include "../../util/inttypes.h"
#include "../../math/math_utils.h"

typedef struct Dimension {
    u8 width;
    u8 height;
} Dimension;

bool dimensionEquals(const Dimension* a, const Dimension* b);

typedef struct Terminator {
    EItemID item;
    Dimension dimension;
} Terminator;

typedef struct RecipeNode {
    EItemID item;
    u8 node_count;
    u8 terminator_count;
    Terminator** terminators;
    struct RecipeNode** nodes; // Must be in item order
} RecipeNode;

typedef EItemID Pattern[9];

RecipeNode* patternNodeGetNext(const RecipeNode* node, const EItemID item);
EItemID patternNodeGetTerminator(const RecipeNode* node, const Dimension* dimension);
EItemID patternTreeSearch(const RecipeNode* root, const Pattern pattern);

#define RECIPE_LIST (RecipeNode*[])
#define RECIPE_ITEM &(RecipeNode)

#define TERMINATOR_LIST (Terminator*[])
#define TERMINATOR_ITEM &(Terminator)

#endif // _PSXMC__GAME_RECIPE__RECIPE_H_
