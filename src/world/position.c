#include "position.h"

#include "../util/math_utils.h"

VECTOR worldToBlockPosition(const VECTOR *position) {
    return (VECTOR){
        position->vx / 100,
        position->vy / 100,
        position->vz / 100
    };
}

VECTOR worldToLocalBlockPosition(const VECTOR *position, const int chunk_size) {
    const VECTOR chunk_position = worldToChunkPosition(position, chunk_size);
    VECTOR block_position = worldToBlockPosition(position);
    block_position.vx -= chunk_position.vx * chunk_size;
    if (chunk_position.vx < 0) block_position.vx--;
    block_position.vy -= chunk_position.vy * chunk_size;
    if (chunk_position.vy < 0) block_position.vy--;
    block_position.vz -= chunk_position.vz * chunk_size;
    if (chunk_position.vz < 0) block_position.vz--;
    return block_position;
}

VECTOR worldToChunkPosition(const VECTOR *position, const int chunk_size) {
    VECTOR chunk_position;
    const int32_t factor = chunk_size * 100;
    chunk_position.vx = position->vx / factor;
    if (position->vx < 0) {
        chunk_position.vx--;
    }
    chunk_position.vy = position->vy / factor;
    if (position->vy < 0) {
        chunk_position.vy--;
    }
    chunk_position.vz = position->vz / factor;
    if (position->vz < 0) {
        chunk_position.vz--;
    }
    return chunk_position;
}

ChunkBlockPosition worldToChunkBlockPosition(const VECTOR* position, const int chunk_size) {
    return (ChunkBlockPosition) {
        (VECTOR) {
            position->vx / chunk_size,
            position->vy / chunk_size,
            position->vz / chunk_size
        },
        (VECTOR) {
            position->vx % chunk_size,
            position->vy % chunk_size,
            position->vz % chunk_size
        }
    };
}

VECTOR chunkBlockToWorldPosition(const ChunkBlockPosition* position,
                                 const int chunk_size) {
    return (VECTOR){
        (position->chunk.vx * chunk_size) + (sign(position->chunk.vx) * position->block.vx),
        (position->chunk.vy * chunk_size) + (sign(position->chunk.vy) * position->block.vy),
        (position->chunk.vz * chunk_size) + (sign(position->chunk.vz) * position->block.vz)
    };
}
