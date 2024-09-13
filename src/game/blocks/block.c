#include "block.h"

void iblockUpdate(VSelf) ALIAS("IBlock_update");
void IBlock_update(VSelf) {
    // Do nothing
}

bool iBlockUseAction(VSelf) ALIAS("IBlock_useAction");
bool IBlock_useAction(VSelf) {
    // By default blocks don't react to being interacted with
    return false;
}

bool iBlockCanPlace(VSelf,
                    const World* world,
                    const VECTOR* position,
                    const AABB* player_aabb) ALIAS("IBlock_canPlace");
bool IBlock_canPlace(VSelf,
                     const World* world,
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
