#include "test_recipe.h"

#include "game/items/items.h"

const RecipeNode* test_recipe = RECIPE_ITEM {
    .item = 0,
    .node_count = 2,
    .result_count = 0,
    .results = NULL,
    .nodes = RECIPE_LIST {
        RECIPE_ITEM {
            .item = 4,
            .node_count = 1,
            .result_count = 0,
            .results = NULL,
            .nodes = RECIPE_LIST {
                RECIPE_ITEM {
                    .item = 4,
                    .node_count = 1,
                    .result_count = 0,
                    .results = NULL,
                    .nodes = RECIPE_LIST {
                        RECIPE_ITEM {
                            .item = 4,
                            .node_count = 1,
                            .result_count = 0,
                            .results = NULL,
                            .nodes = RECIPE_LIST {
                                RECIPE_ITEM {
                                    .item = 4,
                                    .node_count = 1,
                                    .result_count = 0,
                                    .results = NULL,
                                    .nodes = RECIPE_LIST {
                                        RECIPE_ITEM {
                                            .item = 0,
                                            .node_count = 1,
                                            .result_count = 0,
                                            .results = NULL,
                                            .nodes = RECIPE_LIST {
                                                RECIPE_ITEM {
                                                    .item = 4,
                                                    .node_count = 1,
                                                    .result_count = 0,
                                                    .results = NULL,
                                                    .nodes = RECIPE_LIST {
                                                        RECIPE_ITEM {
                                                            .item = 4,
                                                            .node_count = 1,
                                                            .result_count = 0,
                                                            .results = NULL,
                                                            .nodes = RECIPE_LIST {
                                                                RECIPE_ITEM {
                                                                    .item = 4,
                                                                    .node_count = 1,
                                                                    .result_count = 0,
                                                                    .results = NULL,
                                                                    .nodes = RECIPE_LIST {
                                                                        RECIPE_ITEM {
                                                                            .item = 4,
                                                                            .node_count = 0,
                                                                            .result_count = 1,
                                                                            .results = RECIPE_RESULTS_LIST {
                                                                                RECIPE_RESULTS_ITEM {
                                                                                    .dimension = {3, 3},
                                                                                    .result_count = 1,
                                                                                    .results = RECIPE_RESULT_LIST {
                                                                                        RECIPE_RESULT_ITEM {
                                                                                            .item_constructor = itemConstructor(stone),
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
        RECIPE_ITEM {
            .item = 17,
            .node_count = 0,
            .result_count = 1,
            .results = RECIPE_RESULTS_LIST {
                RECIPE_RESULTS_ITEM {
                    .dimension = {1, 1},
                    .result_count = 1,
                    .results = RECIPE_RESULT_LIST {
                        RECIPE_RESULT_ITEM {
                            .item_constructor = itemConstructor(grass),
                            .stack_size = 1,
                        }
                    }
                }
            },
            .nodes = NULL
        }
    }
};