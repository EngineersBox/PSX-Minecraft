#include "circular_buffer.h"

void circularBufferPush(CircularBuffer* buf, const uint8_t data) {
    int next = buf->head + 1;
    if (buf->count >= buf->maxlen) {
        buf->tail = (buf->tail + 1) % buf->maxlen;
        next %= buf->maxlen;
    } else {
        buf->count++;
    }
    buf->buffer[buf->head] = data;
    buf->head = next;
}