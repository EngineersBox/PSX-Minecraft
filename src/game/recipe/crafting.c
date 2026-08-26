#include "crafting.h"

#include <stdbool.h>

#include "../items/items.h"

const RecipeNode* crafting_recipes = RECIPE_ITEM {
    .item = RECIPE_COMPOSITE_ID(ITEMID_AIR, 0),
    .stack_size = 1,
    .ignore_metadata = false,
    .node_count = 2,
    .result_count = 0,
    .processing_ticks = 0,
    .results = NULL,
    .nodes = RECIPE_LIST {
        RECIPE_ITEM {
            .item = RECIPE_COMPOSITE_ID(ITEMID_COBBLESTONE, 0),
            .stack_size = 1,
            .ignore_metadata = false,
            .node_count = 1,
            .result_count = 0,
            .processing_ticks = 0,
            .results = NULL,
            .nodes = RECIPE_LIST {
                RECIPE_ITEM {
                    .item = RECIPE_COMPOSITE_ID(ITEMID_COBBLESTONE, 0),
                    .stack_size = 1,
                    .ignore_metadata = false,
                    .node_count = 1,
                    .result_count = 0,
                    .processing_ticks = 0,
                    .results = NULL,
                    .nodes = RECIPE_LIST {
                        RECIPE_ITEM {
                            .item = RECIPE_COMPOSITE_ID(ITEMID_COBBLESTONE, 0),
                            .stack_size = 1,
                            .ignore_metadata = false,
                            .node_count = 1,
                            .result_count = 0,
                            .processing_ticks = 0,
                            .results = NULL,
                            .nodes = RECIPE_LIST {
                                RECIPE_ITEM {
                                    .item = RECIPE_COMPOSITE_ID(ITEMID_COBBLESTONE, 0),
                                    .stack_size = 1,
                                    .ignore_metadata = false,
                                    .node_count = 0,
                                    .result_count = 1,
                                    .processing_ticks = 1,
                                    .results = RECIPE_RESULTS_LIST {
                                        RECIPE_RESULTS_ITEM {
                                            .dimension = {2, 2},
                                            .result_count = 1,
                                            .results = RECIPE_RESULT_LIST {
                                                RECIPE_RESULT_ITEM {
                                                    .item = RECIPE_COMPOSITE_ID(ITEMID_CRAFTING_TABLE, 0),
                                                    .stack_size = 4,
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
            .item = RECIPE_COMPOSITE_ID(ITEMID_LOG, 0),
            .stack_size = 1,
            .ignore_metadata = true,
            .node_count = 0,
            .result_count = 1,
            .processing_ticks = 20,
            .results = RECIPE_RESULTS_LIST {
                RECIPE_RESULTS_ITEM {
                    .dimension = {1, 1},
                    .result_count = 1,
                    .results = RECIPE_RESULT_LIST {
                        RECIPE_RESULT_ITEM {
                            .item = RECIPE_COMPOSITE_ID(ITEMID_GRASS, 0),
                            .stack_size = 4,
                        }
                    }
                }
            },
            .nodes = NULL
        }
    }
};