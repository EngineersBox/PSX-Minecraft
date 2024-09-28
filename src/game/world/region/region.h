#pragma once

#ifndef _PSXMC__GAME_WORLD_REGION__REGION_H_
#define _PSXMC__GAME_WORLD_REGION__REGION_H_

#include "../../../util/inttypes.h"

#define REGION_RADIUS 2
#define REGION_WIDTH ((REGION_RADIUS * 2) + 1)
#define REGION_CHUNKS_COUNT (REGION_WIDTH * REGION_WIDTH * REGION_WIDTH)

typedef struct RegionHeader {
	// Region location
    struct {
        i8 x;
        i8 y;
        i8 z;
        u8 _pad;
    } position;
    // CRC32 checksum of region data
    u32 crc;
    // Original size of data in bytes
    u32 file_size;
    // Compressed size of data in bytes
    u32 packed_size;
    // Data offset
    u32 offset;
    // Pointer to the next region header;
    u32 next;
} RegionHeader;

typedef struct ChunkHeader {
	// Chunk location
    struct {
        i8 x;
        i8 y;
        i8 z;
        u8 _pad;
    } position;
    // CRC32 checksum of chunk data
    u32 crc;
    // Original size of data in bytes
    u32 file_size;
    // Compressed size of data in bytes
    u32 packed_size;
    // Data offset
    u32 offset;
    // Pointer to the next chunk header;
    u32 next;
} ChunkHeader;

typedef struct RegionArchiveMetadata {
    struct {
        i8 x;
        i8 y;
        i8 z;
    } min;
    struct {
        i8 x;
        i8 y;
        i8 z;
    } max;
    // Data offset
    u32 offset;
} RegionArchiveMetadata;

typedef struct RegionArchive {
    u8 region_count;
    RegionArchiveMetadata metadata[];
} RegionArchive;

#endif // _PSXMC__GAME_WORLD_REGION__REGION_H_
