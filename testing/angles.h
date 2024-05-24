#include <stdio.h>
#include <stdint.h>

#define qN_l	10
#define qN_h	15
#define qA		12
#define B		19900
#define	C		3516

static inline int _isin(int qN, int x) {
	int c, x2, y;

	c  = x << (30 - qN);			// Semi-circle info into carry.
	x -= 1 << qN;					// sine -> cosine calc

	x <<= (31 - qN);				// Mask with PI
	x >>= (31 - qN);				// Note: SIGNED shift! (to qN)
	x  *= x;
	x >>= (2 * qN - 14);			// x=x^2 To Q14

	y = B - (x * C >> 14);			// B - x^2*C
	y = (1 << qA) - (x * y >> 16);	// A - x^2*(B-x^2*C)

	return (c >= 0) ? y : (-y);
}

int isin(int x) {
	return _isin(qN_l, x);
}

int icos(int x) {
	return _isin(qN_l, x + (1 << qN_l));
}

#define FIXED_POINT_SHIFT 12

typedef struct {
    int32_t vx;
    int32_t vy;
    int32_t vz;
} VECTOR;

VECTOR rotationToDirection(const VECTOR* rotation) {
    printf("Rotation: " VEC_PATTERN "\n", rotation->vx, rotation->vy, rotation->vz);
    const int32_t x = rotation->vx >> FIXED_POINT_SHIFT;
    const int32_t y = rotation->vy >> FIXED_POINT_SHIFT;
    const int32_t xz_len = icos(x);
    return (VECTOR) {
        .vx = (xz_len * isin(-y)) >> FIXED_POINT_SHIFT,
        .vy = isin(-x),
        .vz = (xz_len * icos(y)) >> FIXED_POINT_SHIFT
    };
}

// +z => RY = 0
// -z => RY = 4096 / 2 = 2048

// +x => RY = (3 * 4096) / 4 = 3,072
// -x => RY = 4092 / 4 = 1024

// +y => RX = (3 * 4096) / 4 = 3,072
// -y => RX = 4092 / 4 = 1024

int main() {
    VECTOR rotation = (VECTOR) {
        .vx = ((1 * 4096) / 8) << FIXED_POINT_SHIFT,
        .vy = 0 << FIXED_POINT_SHIFT,
    };
    VECTOR direction = rotationToDirection(&rotation);
    printf("Direction: " VEC_PATTERN "\n", direction.vx, direction.vy, direction.vz);
    return 0;
}
