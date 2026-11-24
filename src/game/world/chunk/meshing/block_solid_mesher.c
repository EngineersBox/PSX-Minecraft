#include "block_solid_mesher.h"

#include "meshing.h"

void meshBlockTypeSolid(Chunk* chunk,
                        const PlaneMeshingDataKey* data,
                        const Texture* texture_override,
                        const TextureAttributes* texture_attributes_override,
                        const u32 x,
                        const u32 y,
                        const u32 w,
                        const u32 h) {
    MeshPrimitive* primitive = createPrimitive(
        &chunk->mesh,
        data,
        w,
        h,
        texture_override,
        texture_attributes_override
    );
    createVertices(
        chunk,
        primitive,
        data->face,
        data->axis,
        0,
        x,
        y,
        w,
        h
    );
    createNormal(
        &chunk->mesh,
        primitive,
        data->block,
        data->face
    );
}
