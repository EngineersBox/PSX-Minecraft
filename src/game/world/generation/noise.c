#include "noise.h"

int noise3d(int x, int y, int z) {
    const int X = x >> 16 & 255;
    const int Y = y >> 16 & 255;
    const int Z = z >> 16 & 255;
    const int N = 1 << 16;
    x &= N - 1;
    y &= N - 1;
    z &= N - 1;
    const int u = fade(x);
    const int v = fade(y);
    const int w = fade(z);
    const int A = p[X] + Y;
    const int AA = p[A] + Z;
    const int AB = p[A + 1] + Z;
    const int B = p[X + 1] + Y;
    const int BA = p[B] + Z;
    const int BB = p[B + 1] + Z;
    return lerp(
        w,
        lerp(
            v,
            lerp(
                u,
                grad(p[AA], x, y, z),
                grad(p[BA], x - N, y, z)
            ),
            lerp(
                u,
                grad(p[AB], x, y - N, z),
                grad(p[BB], x - N, y - N, z)
            )
        ),
        lerp(
            v,
            lerp(
                u,
                grad(p[AA + 1], x, y, z - N),
                grad(p[BA + 1], x - N, y, z - N)
            ),
            lerp(
                u,
                grad(p[AB + 1], x, y - N, z - N),
                grad(p[BB + 1], x - N, y - N, z - N)
            )
        )
    );
}

int noise2d(int x, int y) {
    // Find unit square that contains point.
    const int X = x >> 16 & 255;
    const int Y = y >> 16 & 255;
    const int N = 1 << 16;
    // Find relative x,y of point in square.
    x &= N - 1;
    y &= N - 1;

    // Compute fade curves for each of x,y.
    const int u = fade(x);
    const int v = fade(y);
    // Hash coordinates of the corners.
    const int A = p[X] + Y;
    const int AA = p[A];
    const int AB = p[A + 1];
    const int B = p[X + 1] + Y;
    const int BA = p[B];
    const int BB = p[B + 1];
    // Add blended results from the corners.
    return (
        lerp(
            v,
            lerp(
                u,
                grad2d(p[AA], x, y),
                grad2d(p[BA], x - N, y)
            ),
            lerp(
                u,
                grad2d(p[AB], x, y - N),
                grad2d(p[BB], x - N, y - N)
            )
        ) >> 9
    ) + 128;
}

int grad(const int hash, const int x, const int y, const int z) {
    const int h = hash & 15;
    const int u = h < 8 ? x : y;
    const int v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

int grad2d(const int hash, const int x, const int y) {
    const int h = hash & 15;
    const int u = h < 8 ? x : y;
    const int v = h < 4 ? y : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

int fade(const int t) {
    const int t0 = _fade[t >> 8];
    const int t1 = _fade[(t >> 8) + 1];
    return t0 + ((t & 255) * (t1 - t0) >> 8);
}