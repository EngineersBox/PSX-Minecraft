# Region Format

Compressed using a custom variant of the builting LZP library

Each region will be compressed using the inbuilt LZ77 routines, which are part of the LZP library.
The region header will essentially contain the position and data metadata for decompression. Note
the coords are signed 8-bit integer, which allows for `256 * 5 = 1280` chunks per region on a given
axis, and a total of `1280 * 8 = 10240` blocks on each axis per chunk. This makes regions very
dense. Note the use of the `next` field as a pointer to the next region header. This is used so that
regions may be placed anywhere in the cartridge without constraint, which avoids the need to re-write
very large chunks of archives to essentially "de-fragment" an archive to make it readable as a
sequential set of compressed regions.

```c
typedef struct {
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
```
Each region then contains several compressed chunks:

```c
typedef struct {
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
```

A set of regions will be packed in an archive, which has the following structure

```c
typedef struct {
    struct {
        u8 x;
        u8 y;
        u8 z;
    } min;
    struct {
        u8 x;
        u8 y;
        u8 z;
    } max;
    u32 offset;
} RegionArchiveMetadata;

typedef struct {
    u8 region_count;
    RegionArchiveMetadata metadata[];
} GlobalMetadata;
```

This means that an archive can have `2 ^ 8` or `256` region files in it at once. We will create a new
archive each time this threashold is met. It also means more efficient loading and storage. But in
order to know where a given region lies within the archives, we should keep some metadata in a global
header that contains the range of regions and the offsets to the matching archive.

> [!NOTE]
> Need to check if the LZP archive stacks all `LZP_FILE` structures at the start, with the data following
> or if they are staggered with the `offset` and `packed_size` used to index the next file.

