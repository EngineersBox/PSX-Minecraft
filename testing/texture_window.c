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

typedef struct {
    i16 x;
    i16 y;
    i16 w;
    i16 h;
} TWindow;

typedef struct {
    i16 x0;
    i16 y0;
    i16 x1;
    i16 y1;
} MTexWin;

int main() {
    MTexWin m_textureWindow = {0};
    TWindow _prim = {
        .w = 16 >> 3,
        .h = 16 >> 3,
        .x = 0 >> 3,
        .y = 16 >> 3
    };
    TWindow* prim = &_prim;
    // Texture window size is determined by the least bit set of the relevant 5 bits
    if (prim->y & 0x01) {
        m_textureWindow.y1 = 8;  // xxxx1
    } else if (prim->y & 0x02) {
        m_textureWindow.y1 = 16;  // xxx10
    } else if (prim->y & 0x04) {
        m_textureWindow.y1 = 32;  // xx100
    } else if (prim->y & 0x08) {
        m_textureWindow.y1 = 64;  // x1000
    } else if (prim->y & 0x10) {
        m_textureWindow.y1 = 128;  // 10000
    } else {
        m_textureWindow.y1 = 256;  // 00000
    }

    if (prim->x & 0x01) {
        m_textureWindow.x1 = 8;  // xxxx1
    } else if (prim->x & 0x02) {
        m_textureWindow.x1 = 16;  // xxx10
    } else if (prim->x & 0x04) {
        m_textureWindow.x1 = 32;  // xx100
    } else if (prim->x & 0x08) {
        m_textureWindow.x1 = 64;  // x1000
    } else if (prim->x & 0x10) {
        m_textureWindow.x1 = 128;  // 10000
    } else {
        m_textureWindow.x1 = 256;  // 00000
    }

    // Re-calculate the bit field, because we can't trust what is passed in the data
    u32 YAlign = (uint32_t)(32 - (m_textureWindow.y1 >> 3));
    u32 XAlign = (uint32_t)(32 - (m_textureWindow.x1 >> 3));

    // Absolute position of the start of the texture window
    m_textureWindow.y0 = (int16_t)((prim->h & YAlign) << 3);
    m_textureWindow.x0 = (int16_t)((prim->w & XAlign) << 3);
    printf("x0: %d, y0: %d\nx1: %d, y1: %d\n", m_textureWindow.x0, m_textureWindow.y0, m_textureWindow.x1, m_textureWindow.y1);

    return 0;
}
