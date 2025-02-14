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
    u8 stack_size;
    u8 _pad;
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
    /**
     * @brief Recipe item ingredient for this position.
     *        marked by id and metadata id
     */
    CompositeID item;
    u8 stack_size;
    u8 _pad;
    /**
     * @brief Number of elements in `nodes`
     */
    u8 node_count;
    /**
     * @brief Number of elements in `results`
     */
    u8 result_count;
    /**
    * @brief Contains the result of the recipe taking the items in the
    *        the tree up until this node. This should be null when
    *        `result_count`is `NULL`endcode. Otherwise the number of
    *        elements in this array should be equal to `result_count`
    */
    RecipeResults** results;
    /**
    * @brief Next items in the recipe, ordered by item IDs. Note
    *        that this should be null when @code node_count@endcode
    *        is `NULL`. Otherwise the number of elements in this
    *        array should be equal to `node_count`
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

typedef struct RecipePatternEntry {
    CompositeID id;
    u8 stack_size;
    u8 _pad;
} RecipePatternEntry;
typedef RecipePatternEntry* RecipePattern;

#define RECIPE_PATTERN(name, count) RecipePatternEntry name[(count)]

RecipeNode* recipeNodeGetNext(const RecipeNode* node, const RecipePatternEntry* pattern);
// Query result should be initialised with a results
// array within it and the count variable set to the
// size of the array. This is used to verify enough
// space is present when saturating the result in a
// debug build.
RecipeQueryState recipeNodeGetRecipeResult(const RecipeNode* node,
                                           const Dimension* dimension,
                                           RecipeQueryResult* query_result,
                                           bool create_item_result);
// Query result should be initialised with a results
// array within it and the count variable set to the
// size of the array. This is used to verify enough
// space is present when saturating the result in a
// debug build and the create_item_result flag is set.
RecipeQueryState recipeSearch(const RecipeNode* root,
                              const RecipePattern pattern,
                              Dimension pattern_dimension,
                              RecipeQueryResult* query_result,
                              u8* ingredient_consume_sizes,
                              bool create_item_result);

typedef enum RecipeProcessResult {
    RECIPE_PROCESSING_NONE_MATCHING,
    RECIPE_PROCESSING_INSUFFICIENT_SPACE,
    RECIPE_PROCESSING_SUCCEEDED
} RecipeProcessResult;

/** 
 * @brief Find a matching recipe and put the outputs of the recipe
 *        into the provided slots. If the `merge_output` flag is set
 *        then the outputs will increase the stack size of the outputs
 *        if all output slots have room, otherwise no operation is performed.
 * @return Result state indicating what occured (See RecipeProcessResult)
 */
RecipeProcessResult recipeProcessGrid(const RecipeNode* root,
                                      const RecipePattern pattern,
                                      Dimension pattern_dimension,
                                      Slot** output_slots,
                                      u8 output_slot_count,
                                      u8* ingredient_consume_sizes,
                                      bool merge_output);

void recipeConsumeIngredients(Slot* slots,
                              const u8* ingredient_consume_sizes,
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
