#pragma once

#ifndef _PSXMC__GAME_WORLD_CHUNK__HEIGHTMAP_H_
#define _PSXMC__GAME_WORLD_CHUNK__HEIGHTMAP_H_

#include "chunk_defines.h"
#include "../world_defines.h"
#include "../../../util/inttypes.h"

#define chunkHeightmapIndex(x, z) (((x) * CHUNK_SIZE) + z)
#define getChunkHeightmap(Heightmap, x, z) (*heightmap)[chunkHeightmapIndex(x, z)]

typedef u32 ChunkHeightmap[CHUNK_SIZE * CHUNK_SIZE];
typedef ChunkHeightmap Heightmap[AXIS_CHUNKS][AXIS_CHUNKS];

#endif // _PSXMC__GAME_WORLD_CHUNK__HEIGHTMAP_H_
