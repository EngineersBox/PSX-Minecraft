#include "plane_meshing_data.h"

#include "../../../../structure/hashmap.h"
#include "../../../blocks/blocks.h"

int plane_meshing_data_compare(const void* a, const void* b, UNUSED void* ignored) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    const int axis = cmp(pa->key.face, pb->key.face);
    // TODO: We need to account for direction with blocks that have
    //       non-block models or non-uniform face textures
    const bool block_id = blockEquals(pa->key.block, pb->key.block);
    const u16 light_level= cmp(pa->key.light_level, pb->key.light_level);
    const int y = cmp(pa->key.axis, pb->key.axis);
    if (axis != 0) {
        return axis;
    } else if (block_id) {
        return 0;
    } else if (light_level != 0) {
        return light_level;
    }
    return y;
};

u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_xxhash3(&data->key, sizeof(PlaneMeshingDataKey), seed0, seed1);
}
