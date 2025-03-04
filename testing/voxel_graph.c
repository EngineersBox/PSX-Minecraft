#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef int8_t i8;

typedef uint16_t u16;
typedef int16_t i16;

typedef uint32_t u32;
typedef int32_t i32;

typedef uint64_t u64;
typedef int64_t i64;

typedef u8 fixedu8;
typedef i8 fixedi8;

typedef u16 fixedu16;
typedef i16 fixedi16;

typedef u32 fixedu32;
typedef i32 fixedi32;

typedef u64 fixedu64;
typedef i64 fixedi64;

#define CHUNK_SIZE 4
static const u8 VOXELS[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE] = {
    0, 1, 1, 1,
    1, 1, 1, 0,
    1, 1, 1, 0,
    0, 1, 1, 1,

    1, 0, 0, 1,
    0, 0, 1, 1,
    0, 1, 1, 1,
    0, 0, 0, 1,
    
    1, 1, 1, 0,
    1, 1, 0, 1,
    1, 0, 0, 1,
    0, 1, 1, 0,

    1, 1, 1, 1,
    1, 1, 1, 0,
    1, 1, 1, 0,
    0, 1, 1, 0,
};

typedef struct Node {
    u8 cluster_index;
    struct Node** children;
} Node;

int main() {
    return 0;
}
