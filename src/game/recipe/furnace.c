#include "furnace.h"

#include <stdbool.h>

#include "../items/items.h"

const RecipeNode* furnace_recipes = RECIPE_ITEM {
    .id = ITEMID_AIR,
    .metadata_id = 0,
    .stack_size = 1,
    .ignore_metadata = false,
    .node_count = 1,
    .result_count = 0,
    .processing_ticks = 0,
    .results = NULL,
    .nodes = RECIPE_LIST {
        RECIPE_ITEM {
            .id = ITEMID_COBBLESTONE,
            .metadata_id = 0,
            .stack_size = 1,
            .ignore_metadata = false,
            .node_count = 0,
            .result_count = 1,
            .processing_ticks = 200,
            .results = RECIPE_RESULTS_LIST {
                RECIPE_RESULTS_ITEM {
                    .dimension = {1, 1},
                    .result_count = 1,
                    .results = RECIPE_RESULT_LIST {
                        RECIPE_RESULT_ITEM {
                            .id = ITEMID_STONE,
                            .metadata_id = 0,
                            .stack_size = 1,
                        }
                    }
                }
            },
            .nodes = NULL
        }
    }
};