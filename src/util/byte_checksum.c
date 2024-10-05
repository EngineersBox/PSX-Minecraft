#include "byte_checksum.h"

u8 byteChecksum(const u8* data, u32 data_len) {
    u8 checksum = 0;
    for (u32 i = 0; i < data_len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

u8 byteChunkedChecksum(const u8* data, u32 data_len) {
    const u32 chunks = data_len / sizeof(u32);
    const u32 bytes = data_len % sizeof(u32);
    const u32* u32_data = (void*) data;
    u32 u32_checksum = 0;
    for (u32 i = 0; i < chunks; i++) {
        u32_checksum ^= u32_data[i];
    }
    u8 checksum = 0;
    for (u32 i = 0; i < sizeof(u32); i++) {
        checksum ^= (u32_checksum >> (i * 8)) & 0xff;
    }
    const u32 u32_chunks_end = data_len - bytes;
    for (u32 i = 0; i < bytes; i++) {
        checksum ^= data[u32_chunks_end + i];
    }
    return checksum;
}
