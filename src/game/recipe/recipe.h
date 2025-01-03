#pragma once

#ifndef _PSXMC__GAME_RECIPE__RECIPE_H_
#define _PSXMC__GAME_RECIPE__RECIPE_H_

#include <stdbool.h>

#include "../items/item.h"
#include "../items/item_id.h"
#include "../gui/slot.h"
#include "../../util/inttypes.h"
#include "../../math/math_utils.h"

typedef struct Dimension {
    u8 width;
    u8 height;
} Dimension;

bool dimensionEquals(const Dimension* a, const Dimension* b);

typedef union CompositeID {
    struct {
        // Lower bits
        u8 metadata;
        // Higher bits
        EItemID id;
    } separated;
    u16 data;
} CompositeID;

typedef struct RecipeResult {
    /**
     * @brief Recipe item ingredient for this position.
     *        marked by id and metadata id
     */
    CompositeID item;
    u32 stack_size;
} RecipeResult;

typedef struct RecipeResults {
    /**
     * @brief Dimensions of the recipe in a crafting grid, the items in
     *        the tree as parents to this result node are positioned in
     *        the grid from the top left to bottom right. This allows the
     *        recipe to be positioned anywhere and only be constrained to
     *        the pattern itself.
     */
    Dimension dimension;
    u32 result_count;
    RecipeResult** results;
} RecipeResults;

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
 *      and https://www.reddit.com/r/C_Programming/comments/1b5y9r9/compiletime_initialization_of_arbitrarydepth
 */
typedef struct RecipeNode {
    // NOTE: If necessary this could be made into a composite
    //       key that specifies both the id and metadata id
    /**
     * @brief Recipe item ingredient for this position.
     *        marked by id and metadata id
     */
    CompositeID item;
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
    RecipeResults** results;
    /**
    * @brief Next items in the recipe, ordered by item IDs. Note
    *        that this should be null when @code node_count@endcode
    *        is @code NULL@endcode. Otherwise the number of elements
    *        in this array should be equal to @code node_count@endcode
    */
    struct RecipeNode** nodes;
} RecipeNode;

typedef struct RecipeQueryResult {
    u32 result_count;
    IItem** results;
} RecipeQueryResult;

typedef enum ResultQueryState {
    RECIPE_NOT_FOUND,
    RECIPE_FOUND
} RecipeQueryState;

typedef CompositeID RecipePatternEntry;

typedef RecipePatternEntry RecipePattern[9];

RecipeNode* recipeNodeGetNext(const RecipeNode* node, const RecipePatternEntry* pattern);
RecipeQueryState recipeNodeGetRecipeResult(const RecipeNode* node,
                                           const Dimension* dimension,
                                           RecipeQueryResult* query_result,
                                           bool create_item_result);
RecipeQueryState recipeSearch(const RecipeNode* root,
                              const RecipePattern pattern,
                              RecipeQueryResult* query_result,
                              bool create_item_result);

/** 
 * @brief Find a matching recipe and put a single output
 * into the output slot if it is empty. 
 * if no matching recipe
 * @return true if a matching recipe is found, false otherwise
 */
bool recipeProcessGrid(const RecipeNode* node,
                       const RecipePattern pattern,
                       Slot* output_slot);

void recipeConsumeIngredients(Slot* slots,
                              int start_index,
                              int end_index);

#define RECIPE_LIST (RecipeNode*[])
#define RECIPE_ITEM &(RecipeNode)

#define RECIPE_RESULTS_LIST (RecipeResults*[])
#define RECIPE_RESULTS_ITEM &(RecipeResults)

#define RECIPE_RESULT_LIST (RecipeResult*[])
#define RECIPE_RESULT_ITEM &(RecipeResult)

#define RECIPE_COMPOSITE_ID(_id, _metadata) { \
    .separated.metadata = _metadata, \
    .separated.id = _id \
}

#endif // _PSXMC__GAME_RECIPE__RECIPE_H_
