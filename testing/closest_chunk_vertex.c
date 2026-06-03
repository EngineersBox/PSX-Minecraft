#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

/**
 * @brief Compare two numbers and return an integer indicating larger value
 * @param a - first number
 * @param b - second number
 * @return 0 if a == b, 1 if b > a, -1 if b < a
 */
#define cmp(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_b > _a) - (_b < _a); \
})

/**
 * @brief Retrieve the sign (1 for positive, 0 for zero and -1 for negative) of a number
 * @param v - number to apply to
 * @return 1 if v > 0, -1 if v < 0, otherwise 0
 */
#define sign(v) cmp(0, v)

typedef struct Pos {
    int32_t x;
    int32_t y;
} Pos;

int main() {
    for (int32_t x = -1; x <= 2; x++) {
        for (int32_t y = -1; y <= 2; y++) {
            const Pos closest_vertex = (Pos) {
                .x = (x < 0) + x,
                .y = (y < 0) + y
            };
            printf("Pos x: %d y: %d => Vertex x: %d y: %d\n", x, y, closest_vertex.x, closest_vertex.y);
        }
    }
    return 0;
}
