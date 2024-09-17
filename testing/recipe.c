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

typedef struct Terminator {
    EItemId item;
    Dimension dimension;
} Terminator;

typedef struct PatternNode {
    EItemId item;
    u8 node_count;
    u8 terminator_count;
    Terminator** terminators;
    struct PatternNode** nodes; // Must be in item order
} PatternNode;

PatternNode* patternNodeGetNext(const PatternNode* node, const EItemId item) {
    if (node->nodes == NULL || node->node_count == 0) {
        printf("No nodes\n");
        return NULL;
    }
    u32 lower = 0;
    u32 mid;
    u32 upper = node->node_count - 1;
    while (lower <= upper) {
        mid = (lower + upper) >> 1;
        printf("Mid: %d\n", mid);
        PatternNode* next_node = node->nodes[mid];
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

EItemId patternNodeGetTerminator(const PatternNode* node, const Dimension* dimension) {
    if (node->terminators == NULL || node->terminator_count == 0) {
        printf("No terminators\n");
        return NONE;
    }
    for (u32 i = 0; i < node->terminator_count; i++) {
        Terminator* terminator = node->terminators[i];
        printf(
            "Dim a: (%d,%d) Dim b: (%d,%d)\n",
            dimension->width, dimension->height,
            terminator->dimension.width,
            terminator->dimension.height
        );
        if (dimensionEquals(dimension, &terminator->dimension)) {
            return terminator->item;
        }
    }
    return NONE;
}

typedef EItemId Pattern[9];

EItemId patternTreeSearch(const PatternNode* root, const Pattern pattern) {
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
    PatternNode const* current = root;
    for (u8 y = top; y <= bottom; y++) {
        for (u8 x = left; x <= right; x++) {
            current = patternNodeGetNext(current, pattern[(y * 3) + x]);
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
    return patternNodeGetTerminator(current, &dimension);
}

#define PATTERN_LIST (PatternNode*[])
#define PATTERN_ITEM &(PatternNode)

#define TERMINATOR_LIST (Terminator*[])
#define TERMINATOR_ITEM &(Terminator)

const PatternNode* root = PATTERN_ITEM {
    .item = NONE,
    .node_count = 3,
    .terminator_count = 0,
    .terminators = NULL,
    .nodes = PATTERN_LIST {
        [0] = PATTERN_ITEM {
            .item = WOOD,
            .node_count = 1,
            .terminator_count = 0,
            .terminators = NULL,
            .nodes = PATTERN_LIST {
                [0] = PATTERN_ITEM {
                    .item = WOOD,
                    .node_count = 1,
                    .terminator_count = 0,
                    .terminators = NULL,
                    .nodes = PATTERN_LIST {
                        [0] = PATTERN_ITEM {
                            .item = WOOD,
                            .node_count = 1,
                            .terminator_count = 0,
                            .terminators = NULL,
                            .nodes = PATTERN_LIST {
                                [0] = PATTERN_ITEM {
                                    .item = WOOD,
                                    .node_count = 1,
                                    .terminator_count = 0,
                                    .terminators = NULL,
                                    .nodes = PATTERN_LIST {
                                        [0] = PATTERN_ITEM {
                                            .item = WOOD,
                                            .node_count = 1,
                                            .terminator_count = 0,
                                            .terminators = NULL,
                                            .nodes = PATTERN_LIST {
                                                [0] = PATTERN_ITEM {
                                                    .item = WOOD,
                                                    .node_count = 1,
                                                    .terminator_count = 0,
                                                    .terminators = NULL,
                                                    .nodes = PATTERN_LIST {
                                                        [0] = PATTERN_ITEM {
                                                            .item = WOOD,
                                                            .node_count = 1,
                                                            .terminator_count = 0,
                                                            .terminators = NULL,
                                                            .nodes = PATTERN_LIST {
                                                                [0] = PATTERN_ITEM {
                                                                    .item = WOOD,
                                                                    .node_count = 1,
                                                                    .terminator_count = 0,
                                                                    .terminators = NULL,
                                                                    .nodes = PATTERN_LIST {
                                                                        [0] = PATTERN_ITEM {
                                                                            .item = WOOD,
                                                                            .node_count = 0,
                                                                            .terminator_count = 1,
                                                                            .terminators = TERMINATOR_LIST {
                                                                                [0] = TERMINATOR_ITEM {
                                                                                    .item = WOOD_WALL,
                                                                                    .dimension = (Dimension) {3, 3}
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
        [1] = PATTERN_ITEM {
            .item = FLINT,
            .node_count = 1,
            .terminator_count = 0,
            .terminators = NULL,
            .nodes = PATTERN_LIST {
                [0] = PATTERN_ITEM {
                    .item = WOOD,
                    .node_count = 0,
                    .terminator_count = 1,
                    .terminators = TERMINATOR_LIST {
                        [0] = TERMINATOR_ITEM {
                            .item = TORCH,
                            .dimension = (Dimension) {1, 2}
                        }
                    },
                    .nodes = NULL
                }
            }
        },
        [2] = PATTERN_ITEM {
            .item = TORCH,
            .node_count = 1,
            .terminator_count = 0,
            .terminators = NULL,
            .nodes = PATTERN_LIST {
                [0] = PATTERN_ITEM {
                    .item = TORCH,
                    .node_count = 1,
                    .terminator_count = 0,
                    .terminators = NULL,
                    .nodes = PATTERN_LIST {
                        [0] = PATTERN_ITEM {
                            .item = TORCH,
                            .node_count = 0,
                            .terminator_count = 1,
                            .terminators = TERMINATOR_LIST {
                                [0] = TERMINATOR_ITEM {
                                    .item = TORCH_FENCE,
                                    .dimension = (Dimension) {3, 1}
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
    const Pattern pattern = {
        WOOD, WOOD, WOOD,
        WOOD, NONE, WOOD,
        WOOD, WOOD, WOOD
    };
    printf("Search\n");
    EItemId result = patternTreeSearch(root, pattern);
    printf("Result: %d\n", result);
    return 0;
}

