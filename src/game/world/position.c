#include "position.h"

#include "../math/math_utils.h"

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
    // This might seem overly complicated when we can just add
    // together the chunk coords (scaled to block coords) and
    // the block coords. However, doing this allows the user
    // to specify a set of chunk and block coords that do not
    // relate to each other. I.e. the block is not necessarily
    // within the chunk, and still get the correct result.
    const int sign_x = sign(position->chunk.vx);
    const int sign_y = sign(position->chunk.vy);
    const int sign_z = sign(position->chunk.vz);
    return (VECTOR){
        .vx = (position->chunk.vx * chunk_size) + (sign_x * positiveModulo(chunk_size + (sign_x * position->block.vx), chunk_size)),
        .vy = (position->chunk.vy * chunk_size) + (sign_y * positiveModulo(chunk_size + (sign_y * position->block.vy), chunk_size)),
        .vz = (position->chunk.vz * chunk_size) + (sign_z * positiveModulo(chunk_size + (sign_z * position->block.vz), chunk_size))
    };
}
