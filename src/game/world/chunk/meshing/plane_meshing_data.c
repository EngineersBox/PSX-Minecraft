#include "plane_meshing_data.h"

#include "../../../structure/hashmap.h"

int plane_meshing_data_compare(const void* a, const void* b, void* ignored) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    const int axis = cmp(pa->key.face, pb->key.face);
    const int blockId = cmp(pa->key.block->id, pb->key.block->id);
    const int y = cmp(pa->key.axis, pb->key.axis);
    if (axis != 0) {
        return axis;
    } else if (blockId != 0) {
        return blockId;
    }
    return y;
};

u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_xxhash3(&data->key, sizeof(PlaneMeshingDataKey), seed0, seed1);
}
