#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CHUNK_SIZE 4

int main() {
    int i = 0;
    for (int y = 0; y < CHUNK_SIZE; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {    
                if ((x + y + z) % 2 != 0) {
                    printf("(%d, %d, %d)\n", CHUNK_SIZE - 1 - x, CHUNK_SIZE - 1 - y, CHUNK_SIZE - 1 - z);
                } else {
                    printf("(%d, %d, %d)\n", x, y, z);
                }
                i++;
            }
        }
    }
    return 0;
}
