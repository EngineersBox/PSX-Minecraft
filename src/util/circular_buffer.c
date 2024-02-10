#include "circular_buffer.h"

void circularBufferPush(CircularBuffer* buf, const uint8_t data) {
    int next = buf->head + 1;
    if (next >= buf->maxlen) {
        next = 0;
    }
    if (next == buf->tail) {
        buf->tail++;
    }
    buf->buffer[buf->head] = data;
    buf->head = next;
}