#pragma once

#ifndef PSXMC_CIRCULAR_BUFFER_H
#define PSXMC_CIRCULAR_BUFFER_H

#include <stdint.h>

typedef struct {
    uint8_t* const buffer;
    volatile int head;
    volatile int tail;
    volatile int count;
    const int maxlen;
} CircularBuffer;

#define DEFINE_CIRCULAR_BUFFER(name, size) \
        uint8_t name##_data[(size)]; \
    CircularBuffer name = { \
        .buffer = name##_data, \
        .head = 0, \
        .tail = 0, \
        .count = 0, \
        .maxlen = (size) \
    }

void circularBufferPush(CircularBuffer* buf, uint8_t data);

#endif // PSXMC_CIRCULAR_BUFFER_H
