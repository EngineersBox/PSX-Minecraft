#include "block.h"

BlockInputHandlerContext block_input_handler_context = {
    .inventory = NULL,
    .block = NULL
};
BlockRenderUIContext block_render_ui_context = {
    .function = NULL,
    .block = NULL
};

void iblockUpdate(VSelf) ALIAS("IBlock_update");
void IBlock_update(UNUSED VSelf) {
    // Do nothing
}

bool iBlockUseAction(VSelf) ALIAS("IBlock_useAction");
bool IBlock_useAction(UNUSED VSelf) {
    // By default blocks don't react to being interacted with
    return BLOCK_USE_ACTION_NOT_CONSUMED;
}

bool iBlockCanPlace(VSelf,
                    const World* world,
                    const VECTOR* position,
                    const AABB* player_aabb) ALIAS("IBlock_canPlace");
bool IBlock_canPlace(UNUSED VSelf,
                     UNUSED const World* world,
                     const VECTOR* position,
                     const AABB* player_aabb) {
    const AABB aabb = (AABB) {
        .min = vec3_const_mul(*position, ONE_BLOCK),
        .max = vec3_const_mul(
            vec3_const_add(*position, 1),
            ONE_BLOCK
        )
    };
    return !aabbIntersects(
        player_aabb,
        &aabb
    );
}

void iBlockRenderUI(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("IBlock_renderUI");
void IBlock_renderUI(UNUSED VSelf,
                     UNUSED RenderContext* ctx,
                     UNUSED Transforms* transforms) {
    // Do nothing
}
