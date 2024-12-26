#pragma once

#ifndef _PSXMC__GAME_RECIPE__CRAFTING_TABLE_RECIPES_H_
#define _PSXMC__GAME_RECIPE__CRAFTING_TABLE_RECIPES_H_

#include "recipe.h"
#include "../items/item_id.h"

const RecipeNode* crafting_table_recipes = RECIPE_ITEM {
    .item = ITEMID_AIR,
    .node_count = 0,
    .result_count = 0,
    .results = NULL,
    .nodes = RECIPE_LIST {}
};

#endif // _PSXMC__GAME_RECIPE__CRAFTING_TABLE_RECIPES_H_
