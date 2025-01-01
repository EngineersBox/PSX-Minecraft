#include "recipe.h"

#include "../../util/interface99_extensions.h"
#include "../items/items.h"

INLINE bool dimensionEquals(const Dimension* a, const Dimension* b) {
    return a->width == b->width && a->height == b->height; 
}

RecipeNode* recipeNodeGetNext(const RecipeNode* node, const RecipePatternEntry* pattern) {
    if (node->nodes == NULL || node->node_count == 0) {
        return NULL;
    }
    // NOTE: These need to be signed otherwise we can get upper to
    // be u32::MAX if there is only one element in the array and it
    // doesn't match the item
    i32 lower = 0;
    i32 mid;
    i32 upper = node->node_count - 1;
    while (lower <= upper) {
        mid = (lower + upper) >> 1;
        RecipeNode* next_node = node->nodes[mid];
        if (next_node->item.data == pattern->data) {
            return next_node;
        } else if (next_node->item.data > pattern->data) {
            upper = mid - 1;
        } else {
            lower = mid + 1;
        }
    }
    return NULL;
}

static void assembleResult(const RecipeResults* results, RecipeQueryResult* query_result) {
    query_result->result_count = results->result_count;
    for (u32 i = 0; i < results->result_count; i++) {
        const IItem* existing_iitem = query_result->results[i];
        const RecipeResult* result = results->results[i];
        if (existing_iitem == NULL) {
            goto assemble_new_item;
        }
        Item* existing_item = VCAST_PTR(Item*, existing_iitem);
        if (itemIdEquals(existing_item, result->item.separated.id, result->item.separated.metadata)) {
            // Resize stack
            existing_item->stack_size = result->stack_size;
            continue;
        }
        // Replace item in result with new one
assemble_new_item:;
        IItem* iitem = itemGetConstructor(result->item.separated.id)(result->item.separated.metadata);
        assert(iitem != NULL);
        Item* item = VCAST_PTR(Item*, iitem);
        itemSetWorldState(item, false);
        VCALL_SUPER(*iitem, Renderable, applyInventoryRenderAttributes);
        item->bob_offset = 1;
        item->stack_size = result->stack_size;
        query_result->results[i] = iitem;
    }
}

RecipeQueryState recipeNodeGetRecipeResult(const RecipeNode* node,
                                           const Dimension* dimension,
                                           RecipeQueryResult* query_result,
                                           bool create_result_item) {
    if (node->results == NULL || node->result_count == 0) {
        return RECIPE_NOT_FOUND; 
    }
    for (u32 i = 0; i < node->result_count; i++) {
        RecipeResults* result = node->results[i];
        if (dimensionEquals(dimension, &result->dimension)) {
            if (create_result_item) assembleResult(result, query_result);
            return RECIPE_FOUND;
        }
    }
    return RECIPE_NOT_FOUND;
}

RecipeQueryState recipeSearch(const RecipeNode* root,
                              const RecipePattern pattern,
                              RecipeQueryResult* query_result,
                              bool create_result_item) {
    u8 right = 0;
    u8 bottom = 0;
    u8 top = 3;
    u8 left = 3;
    for (u8 y = 0; y < 3; y++) {
        for (u8 x = 0; x < 3; x++) {
            if (pattern[(y * 3) + x].separated.id != ITEMID_AIR) {
                left = min(left, x);
                top = min(top, y);
                right = max(right, x);
                bottom = max(bottom, y);
            }
        }
    }
    RecipeNode const* current = root;
    for (u8 y = top; y <= bottom; y++) {
        for (u8 x = left; x <= right; x++) {
            current = recipeNodeGetNext(current, &pattern[(y * 3) + x]);
            if (current == NULL) {
                return RECIPE_NOT_FOUND;
            }
        }
    }
    const Dimension dimension = (Dimension) {
        .width = right - left + 1,
        .height = bottom - top + 1
    };
    return recipeNodeGetRecipeResult(
        current,
        &dimension,
        query_result,
        create_result_item
    );
}
