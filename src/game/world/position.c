#include "position.h"

#include "../../logging/logging.h"
#include "../../math/vector.h"
#include "../../math/math_utils.h"

ChunkBlockPosition worldToChunkBlockPosition(const VECTOR* position, const int chunk_size) {
    return (ChunkBlockPosition) {
        .chunk = (VECTOR) {
            .vx = floorDiv(position->vx, chunk_size),
            .vy = floorDiv(position->vy, chunk_size),
            .vz = floorDiv(position->vz, chunk_size)
        },
        .block = (VECTOR) {
            .vx = positiveModulo(position->vx, chunk_size),
            .vy = positiveModulo(position->vy, chunk_size),
            .vz = positiveModulo(position->vz, chunk_size)
        }
    };
}

VECTOR chunkBlockToWorldPosition(const ChunkBlockPosition* position,
                                 const int chunk_size) {
    // This allows the user to specify a set of chunk and block
    // coords that do not relate to each other. I.e. the block
    // is not necessarily within the chunk, and still get the
    // correct result.
    return vec3_i32(
        (position->chunk.vx * chunk_size) + position->block.vx,
        (position->chunk.vy * chunk_size) + position->block.vy,
        (position->chunk.vz * chunk_size) + position->block.vz
    );
}
