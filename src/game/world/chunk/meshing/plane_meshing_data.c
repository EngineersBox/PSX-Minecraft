#include "plane_meshing_data.h"

#include "../../../../structure/hashmap.h"
#include "../../../blocks/blocks.h"

int plane_meshing_data_compare(const void* a, const void* b, UNUSED void* ignored) {
    const PlaneMeshingData* pa = a;
    const PlaneMeshingData* pb = b;
    // NOTE: Extracing fields into variables is necessary
    //       to avoid sizeof_alignof_typeof_bitfield
    const FaceDirection face_a = pa->key.face;
    const FaceDirection face_b = pb->key.face;
    const int axis = cmp(face_a, face_b);
    if (axis != 0) {
        // Different faces being meshed
        return axis;
    }
    // NOTE: Accounts for direction with blocks that have
    //       non-block models or non-uniform face textures
    const FaceDirection orientation_a = pa->key.block->orientation;
    const FaceDirection orientation_b = pb->key.block->orientation;
    const int orientation = cmp(orientation_a, orientation_b);
    if (blockEquals(pa->key.block, pb->key.block) || orientation != 0) {
        // Same block with different orientations or different block
        // or different orientation
        return orientation;
    }
    const u16 light_level= cmp(pa->key.light_level, pb->key.light_level);
    if (light_level != 0) {
        // Different light levels
        return light_level;
    }
    // Final decider is the axis we are meshing across
    const FaceDirection axis_a = pa->key.axis;
    const FaceDirection axis_b = pb->key.axis;
    return cmp(axis_a, axis_b);
};

u64 plane_meshing_data_hash(const void* item, u64 seed0, u64 seed1) {
    const PlaneMeshingData* data = item;
    return hashmap_xxhash3(&data->key, sizeof(PlaneMeshingDataKey), seed0, seed1);
}
