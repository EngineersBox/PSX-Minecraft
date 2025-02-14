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
    // Ensure that we have enough space to saturate the result
    assert(query_result->result_count == results->result_count);
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

bool sufficientSpaceInOutputSlots(const RecipeQueryResult* query_result,
                                  Slot** output_slots,
                                  const u8 output_slot_count) {
    for (u8 i = 0; i < output_slot_count; i++) {
        const Item* output_item = VCAST_PTR(Item*, output_slots[i]->data.item);
        const Item* result_item = VCAST_PTR(Item*, query_result->results[i]);
        if (itemGetMaxStackSize(output_item->id) - output_item->stack_size < result_item->stack_size) {
            return false;
        }
    }
    return true;
}

RecipeProcessResult recipeProcessGrid(const RecipeNode* root,
                                      const RecipePattern pattern,
                                      Slot** output_slots,
                                      u8 output_slot_count,
                                      bool merge_output) {
    RecipeQueryResult query_result = {0};
    query_result.results = calloc(output_slot_count, sizeof(IItem*));
    assert(query_result.results != NULL);
    query_result.result_count = output_slot_count;
    // Put existing output in the result array
    // to use when determining if the stack should
    // just be resized or we need to create a new
    // item if the IDs are different.
    for (u8 i = 0; i < output_slot_count; i++) {
        query_result.results[i] = output_slots[i]->data.item;
    }
    if (recipeSearch(
        root,
        pattern,
        &query_result,
        true
    ) == RECIPE_NOT_FOUND) {
        // No matching recipe
        free(query_result.results);
        return RECIPE_PROCESSING_NONE_MATCHING;
    }
    if (!merge_output && !sufficientSpaceInOutputSlots(
        &query_result,
        output_slots,
        output_slot_count
    )) {
        for (u8 i = 0; i < output_slot_count; i++) {
            free(query_result.results[i]);
        }
        return RECIPE_PROCESSING_INSUFFICIENT_SPACE;
    }
    for (u8 i = 0; i < output_slot_count; i++) {
        Slot* output_slot = output_slots[i];
        if (output_slot->data.item == NULL) {
            // Output slot was empty, just move the result
            // into it
            goto free_result_and_move_to_slot;
        }
        Item* output_item = VCAST_PTR(Item*, output_slot->data.item);
        const Item* result_item = VCAST_PTR(Item*, query_result.results[i]);
        if (itemEquals(output_item, result_item)) {
            // Items were the same, stack size was adjusted
            // we have nothing left to do
            if (!merge_output) {
                goto free_result;
            }
            output_item->stack_size += result_item->stack_size;
            goto free_result;
        }
        // IDs between existing output slot item
        // and new result were different, 
        VCALL((IItem) *output_slot->data.item, destroy);
    free_result_and_move_to_slot:;
        output_slot->data.item = query_result.results[i];
    free_result:;
        free(query_result.results[i]);
    }
    return RECIPE_PROCESSING_SUCCEEDED;
}

void recipeConsumeIngredients(Slot* slots,
                              int start_index,
                              int end_index) {
    for (int i = start_index; i < end_index; i++) {
        Slot* slot = &slots[i];
        IItem* iitem = slot->data.item;
        if (iitem == NULL) {
            continue;
        }
        Item* item = VCAST_PTR(Item*, iitem);
        assert(item->stack_size > 0);
        if (--item->stack_size == 0) {
            VCALL(*iitem, destroy);
            slot->data.item = NULL;
        }
    }
}
