#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef int8_t i8;

typedef uint16_t u16;
typedef int16_t i16;

typedef uint32_t u32;
typedef int32_t i32;

typedef uint64_t u64;
typedef int64_t i64;

typedef u8 fixedu8;
typedef i8 fixedi8;

typedef u16 fixedu16;
typedef i16 fixedi16;

typedef u32 fixedu32;
typedef i32 fixedi32;

typedef u64 fixedu64;
typedef i64 fixedi64;

#define max(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b; \
})

#define min(a,b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b; \
})

typedef struct Dimension {
    u8 width;
    u8 height;
} Dimension;

inline static bool dimensionEquals(const Dimension* a, const Dimension* b) {
    return a->width == b->width && a->height == b->height; 
}

typedef enum EItemID {
    NONE = 0,
    WOOD = 1,
    FLINT = 2,
    TORCH = 3,
    WOOD_WALL = 4,
    TORCH_FENCE = 5
} EItemId;

typedef struct RecipeResult {
    EItemId item;
    Dimension dimension;
} RecipeResult;

typedef struct RecipeNode {
    EItemId item;
    u8 node_count;
    u8 result_count;
    RecipeResult** results;
    struct RecipeNode** nodes; // Must be in item order
} RecipeNode;

RecipeNode* recipeNodeGetNext(const RecipeNode* node, const EItemId item) {
    if (node->nodes == NULL || node->node_count == 0) {
        printf("No nodes\n");
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
        printf("Mid: %d\n", mid);
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

EItemId recipeNodeGetRecipeResult(const RecipeNode* node, const Dimension* dimension) {
    if (node->results == NULL || node->result_count == 0) {
        printf("No results\n");
        return NONE;
    }
    for (u32 i = 0; i < node->result_count; i++) {
        RecipeResult* result = node->results[i];
        printf(
            "Dim a: (%d,%d) Dim b: (%d,%d)\n",
            dimension->width, dimension->height,
            result->dimension.width,
            result->dimension.height
        );
        if (dimensionEquals(dimension, &result->dimension)) {
            return result->item;
        }
    }
    return NONE;
}

typedef EItemId RecipePattern[9];

EItemId recipeSearch(const RecipeNode* root, const RecipePattern pattern) {
    u8 right = 0;
    u8 bottom = 0;
    u8 top = 3;
    u8 left = 3;
    for (u8 y = 0; y < 3; y++) {
        for (u8 x = 0; x < 3; x++) {
            if (pattern[(y * 3) + x] != NONE) {
                left = min(left, x);
                top = min(top, y);
                right = max(right, x);
                bottom = max(bottom, y);
            }
        }
    }
    printf("Right: %d Bottom: %d Top: %d Left: %d\n", right, bottom, top, left);
    RecipeNode const* current = root;
    for (u8 y = top; y <= bottom; y++) {
        for (u8 x = left; x <= right; x++) {
            current = recipeNodeGetNext(current, pattern[(y * 3) + x]);
            printf("Next: %d\n", current);
            if (current == NULL) {
                return NONE;
            }
        }
    }
    const Dimension dimension = (Dimension) {
        .width = right - left + 1,
        .height = bottom - top + 1
    };
    return recipeNodeGetRecipeResult(current, &dimension);
}

#define RECIPE_LIST (RecipeNode*[])
#define RECIPE_ITEM &(RecipeNode)

#define RECIPE_RESULT_LIST (RecipeResult*[])
#define RECIPE_RESULT_ITEM &(RecipeResult)

const RecipeNode* root = RECIPE_ITEM {
    .item = NONE,
    .node_count = 3,
    .result_count = 0,
    .results = NULL,
    .nodes = RECIPE_LIST {
        [0] = RECIPE_ITEM {
            .item = WOOD,
            .node_count = 1,
            .result_count = 0,
            .results = NULL,
            .nodes = RECIPE_LIST {
                [0] = RECIPE_ITEM {
                    .item = WOOD,
                    .node_count = 1,
                    .result_count = 0,
                    .results = NULL,
                    .nodes = RECIPE_LIST {
                        [0] = RECIPE_ITEM {
                            .item = WOOD,
                            .node_count = 1,
                            .result_count = 0,
                            .results = NULL,
                            .nodes = RECIPE_LIST {
                                [0] = RECIPE_ITEM {
                                    .item = WOOD,
                                    .node_count = 1,
                                    .result_count = 0,
                                    .results = NULL,
                                    .nodes = RECIPE_LIST {
                                        [0] = RECIPE_ITEM {
                                            .item = WOOD,
                                            .node_count = 1,
                                            .result_count = 0,
                                            .results = NULL,
                                            .nodes = RECIPE_LIST {
                                                [0] = RECIPE_ITEM {
                                                    .item = WOOD,
                                                    .node_count = 1,
                                                    .result_count = 0,
                                                    .results = NULL,
                                                    .nodes = RECIPE_LIST {
                                                        [0] = RECIPE_ITEM {
                                                            .item = WOOD,
                                                            .node_count = 1,
                                                            .result_count = 0,
                                                            .results = NULL,
                                                            .nodes = RECIPE_LIST {
                                                                [0] = RECIPE_ITEM {
                                                                    .item = WOOD,
                                                                    .node_count = 1,
                                                                    .result_count = 0,
                                                                    .results = NULL,
                                                                    .nodes = RECIPE_LIST {
                                                                        [0] = RECIPE_ITEM {
                                                                            .item = WOOD,
                                                                            .node_count = 0,
                                                                            .result_count = 1,
                                                                            .results = RECIPE_RESULT_LIST {
                                                                                [0] = RECIPE_RESULT_ITEM {
                                                                                    .item = WOOD_WALL,
                                                                                    .dimension = {3, 3}
                                                                                }
                                                                            },
                                                                            .nodes = NULL
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        },
        [1] = RECIPE_ITEM {
            .item = FLINT,
            .node_count = 1,
            .result_count = 0,
            .results = NULL,
            .nodes = RECIPE_LIST {
                [0] = RECIPE_ITEM {
                    .item = WOOD,
                    .node_count = 0,
                    .result_count = 1,
                    .results = RECIPE_RESULT_LIST {
                        [0] = RECIPE_RESULT_ITEM {
                            .item = TORCH,
                            .dimension = {1, 2}
                        }
                    },
                    .nodes = NULL
                }
            }
        },
        [2] = RECIPE_ITEM {
            .item = TORCH,
            .node_count = 1,
            .result_count = 0,
            .results = NULL,
            .nodes = RECIPE_LIST {
                [0] = RECIPE_ITEM {
                    .item = TORCH,
                    .node_count = 1,
                    .result_count = 0,
                    .results = NULL,
                    .nodes = RECIPE_LIST {
                        [0] = RECIPE_ITEM {
                            .item = TORCH,
                            .node_count = 0,
                            .result_count = 1,
                            .results = RECIPE_RESULT_LIST {
                                [0] = RECIPE_RESULT_ITEM {
                                    .item = TORCH_FENCE,
                                    .dimension = {3, 1}
                                }
                            },
                            .nodes = NULL
                        }
                    }
                }
            }
        }
    }
};

int main() {
    const RecipePattern pattern = {
        WOOD, WOOD, WOOD,
        WOOD, NONE, WOOD,
        WOOD, WOOD, WOOD
    };
    printf("Search\n");
    EItemId result = recipeSearch(root, pattern);
    printf("Result: %d\n", result);
    return 0;
}

