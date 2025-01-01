#include "crafting_table.h"

#include "../items/items.h"

const RecipeNode* crafting_table_recipes = RECIPE_ITEM {
    .item = RECIPE_COMPOSITE_ID(0, 0),
    .node_count = 2,
    .result_count = 0,
    .results = NULL,
    .nodes = RECIPE_LIST {
        RECIPE_ITEM {
            .item = RECIPE_COMPOSITE_ID(5, 0),
            .node_count = 1,
            .result_count = 0,
            .results = NULL,
            .nodes = RECIPE_LIST {
                RECIPE_ITEM {
                    .item = RECIPE_COMPOSITE_ID(5, 0),
                    .node_count = 1,
                    .result_count = 0,
                    .results = NULL,
                    .nodes = RECIPE_LIST {
                        RECIPE_ITEM {
                            .item = RECIPE_COMPOSITE_ID(5, 0),
                            .node_count = 1,
                            .result_count = 0,
                            .results = NULL,
                            .nodes = RECIPE_LIST {
                                RECIPE_ITEM {
                                    .item = RECIPE_COMPOSITE_ID(5, 0),
                                    .node_count = 0,
                                    .result_count = 1,
                                    .results = RECIPE_RESULTS_LIST {
                                        RECIPE_RESULTS_ITEM {
                                            .dimension = {2, 2},
                                            .result_count = 1,
                                            .results = RECIPE_RESULT_LIST {
                                                RECIPE_RESULT_ITEM {
                                                    .item = RECIPE_COMPOSITE_ID(5, 0),
                                                    .stack_size = 1,
                                                }
                                            }
                                        }
                                    },
                                    .nodes = NULL
                                }
                            }
                        }
                    }
                }
            }
        },
        RECIPE_ITEM {
            .item = RECIPE_COMPOSITE_ID(17, 0),
            .node_count = 0,
            .result_count = 1,
            .results = RECIPE_RESULTS_LIST {
                RECIPE_RESULTS_ITEM {
                    .dimension = {1, 1},
                    .result_count = 1,
                    .results = RECIPE_RESULT_LIST {
                        RECIPE_RESULT_ITEM {
                            .item = RECIPE_COMPOSITE_ID(17, 0),
                            .stack_size = 4,
                        }
                    }
                }
            },
            .nodes = NULL
        }
    }
};