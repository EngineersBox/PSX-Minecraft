#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

typedef int32_t i32;

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

static const Pos quadrant_verts[4][3] = {
    [0]={
        [0]=(Pos) { .x = 0, .y = 0 },
        [1]=(Pos) { .x = 1, .y = 0 },
        [2]=(Pos) { .x = 0, .y = 1 },
    },
    [1]={
        [0]=(Pos) { .x = 1, .y = 0 },
        [1]=(Pos) { .x = 1, .y = 1 },
        [2]=(Pos) { .x = 0, .y = 0 },
    },
    [2]={
        [0]=(Pos) { .x = 0, .y = 1 },
        [1]=(Pos) { .x = 0, .y = 0 },
        [2]=(Pos) { .x = 1, .y = 1 },
    },
    [3]={
        [0]=(Pos) { .x = 1, .y = 1 },
        [1]=(Pos) { .x = 0, .y = 1 },
        [2]=(Pos) { .x = 1, .y = 0 },
    },
};

int main() {
    for (int32_t x = -1; x <= 1; x++) {
        for (int32_t y = -1; y <= 1; y++) {
            if (x == 0 || y == 0) {
                printf("[%d,%d] Covered by centre check\n", x, y);
                continue;
            }
            const size_t quadrant = (y > 0 ? 0 : 2) + (x > 0 ? 0 : 1);
            printf("Quadrant: %d\n", quadrant);
            const Pos* quadrant_vert = quadrant_verts[quadrant];
            for (size_t i = 0; i < 3; i++) {
                const Pos pos = (Pos) {
                    .x = (8 * x) + (8 * quadrant_vert[i].x),
                    .y = (8 * y) + (8 * quadrant_vert[i].y)
                };
                printf("[%d,%d] Check (%d,%d)\n", x, y, pos.x, pos.y);
            }
        }
    }
    return 0;
}
