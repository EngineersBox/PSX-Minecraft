#include "recipe.h"

INLINE bool dimensionEquals(const Dimension* a, const Dimension* b) {
    return a->width == b->width && a->height == b->height; 
}

RecipeNode* patternNodeGetNext(const RecipeNode* node, const EItemID item) {
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
        if (next_node->item == item) {
            return next_node;
        } else if (next_node->item > item) {
            upper = mid - 1;
        } else {
            lower = mid + 1;
        }
    }
    return NULL;
}

EItemID patternNodeGetTerminator(const RecipeNode* node, const Dimension* dimension) {
    if (node->terminators == NULL || node->terminator_count == 0) {
        return ITEMID_AIR;
    }
    for (u32 i = 0; i < node->terminator_count; i++) {
        Terminator* terminator = node->terminators[i];
        if (dimensionEquals(dimension, &terminator->dimension)) {
            return terminator->item;
        }
    }
    return ITEMID_AIR;
}

EItemID patternTreeSearch(const RecipeNode* root, const Pattern pattern) {
    u8 right = 0;
    u8 bottom = 0;
    u8 top = 3;
    u8 left = 3;
    for (u8 y = 0; y < 3; y++) {
        for (u8 x = 0; x < 3; x++) {
            if (pattern[(y * 3) + x] != ITEMID_AIR) {
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
            current = patternNodeGetNext(current, pattern[(y * 3) + x]);
            if (current == NULL) {
                return ITEMID_AIR;
            }
        }
    }
    const Dimension dimension = (Dimension) {
        .width = right - left + 1,
        .height = bottom - top + 1
    };
    return patternNodeGetTerminator(current, &dimension);
}
