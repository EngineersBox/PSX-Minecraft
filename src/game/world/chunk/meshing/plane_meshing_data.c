#include "plane_meshing_data.h"

#include "../../../structure/hashmap.h"

int plane_meshing_data_compare(const void* a, const void* b, void* ignored) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    const int axis = cmp(pa->key.face, pb->key.face);
    const int block_id = cmp(pa->key.block->id, pb->key.block->id);
    const u16 light_level_colour = cmp(pa->key.light_level_colour_scalar, pb->key.light_level_colour_scalar);
    const int y = cmp(pa->key.axis, pb->key.axis);
    if (axis != 0) {
        return axis;
    } else if (block_id != 0) {
        return block_id;
    } else if (light_level_colour != 0) {
        return light_level_colour;
    }
    return y;
};

u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_xxhash3(&data->key, sizeof(PlaneMeshingDataKey), seed0, seed1);
}
