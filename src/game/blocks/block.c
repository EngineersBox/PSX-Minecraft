#include "block.h"

#include <psxgte.h>

#include "../../util/memory.h"
#include "../../structure/hashmap.h"

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

IBlock* iblockCreate() {
    IBlock* iblock = malloc(sizeof(IBlock));
    zeroed(iblock);
    return iblock;
}

void iblockDestroy(IBlock* iblock) {
    free(iblock);
}

u64 blockUpdateHash(const void* item, u64 seed0, u64 seed1) {
    const BlockUpdate* block_update = item;
    return hashmap_xxhash3(
        &block_update->position,
        sizeof(VECTOR),
        seed0,
        seed1
    );
}

int blockUpdateCompare(const void* a, const void* b, UNUSED void* ignored) {
    const BlockUpdate* block_update_a = a;
    const BlockUpdate* block_update_b = b;
    // Negation here since this compare function is like the
    // cmp(..) function in the standard library, where a return
    // value of 0 implies equivalence.
    return !vec3_equal(block_update_a->position, block_update_b->position);
}
