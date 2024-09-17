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

typedef struct RecipeResult {
    EItemID item;
    Dimension dimension;
} RecipeResult;

/**
 * @brief Represents a recipe item ingredient or a result based on
 *        the items in the tree until this point. A result for this
 *        pattern is stored as a result. A recipe pattern heirarchy
 *        should be constructed in order of left-to-right, top-to-bottom
 *        in terms of node heirarchy. Only the nodes required to fully
 *        encapsulate the recipe are requried (i.e if it fits within a
 *        3x2 area then you only need to include those nodes and ignore
 *        the 3x1 area that is unusesd). This format allows for recipes
 *        to be anywhere in the grid and still be matched.
 * @see This is based on @link https://gamedev.stackexchange.com/a/21626
 *      and https://www.reddit.com/r/C_Programming/comments/1b5y9r9/compiletime_initialization_of_arbitrarydepth/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
 */
typedef struct RecipeNode {
    /**
     * @brief Recipe item ingredient for this position.
     */
    EItemID item;
    /**
     * @brief Number of elements in @code nodes@endcode
     */
    u8 node_count;
    /**
     * @brief Number of elements in @code results@endcode
     */
    u8 result_count;
    /**
    * @brief Contains the result of the recipe taking the items in the
    *        the tree up until this node. This should be null when
    *        @code result_count@endcode is @code NULL@endcode.
    *        Otherwise the number of elements in this array should be
    *        equal to @code result_count@endcode
    */
    RecipeResult** results;
    /**
    * @brief Next items in the recipe, ordered by item IDs. Note
    *        that this should be null when @code node_count@endcode
    *        is @code NULL@endcode. Otherwise the number of elements
    *        in this array should be equal to @code node_count@endcode
    */
    struct RecipeNode** nodes;
} RecipeNode;

typedef EItemID RecipePattern[9];

RecipeNode* recipeNodeGetNext(const RecipeNode* node, const EItemID item);
EItemID recipeNodeGetRecipeResult(const RecipeNode* node, const Dimension* dimension);
EItemID recipeSearch(const RecipeNode* root, const RecipePattern pattern);

#define RECIPE_LIST (RecipeNode*[])
#define RECIPE_ITEM &(RecipeNode)

#define RECIPE_RESULT_LIST (RecipeResult*[])
#define RECIPE_RESULT_ITEM &(RecipeResult)

#endif // _PSXMC__GAME_RECIPE__RECIPE_H_
