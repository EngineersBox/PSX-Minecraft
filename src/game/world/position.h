#pragma once

#ifndef PSX_MINECRAFT_POSITION_H
#define PSX_MINECRAFT_POSITION_H

#include <psxgte.h>

typedef struct {
    VECTOR chunk;
    VECTOR block;
} ChunkBlockPosition;

ChunkBlockPosition worldToChunkBlockPosition(const VECTOR* position, int chunk_size);
VECTOR chunkBlockToWorldPosition(const ChunkBlockPosition* position, int chunk_size);

#endif // PSX_MINECRAFT_POSITION_H
