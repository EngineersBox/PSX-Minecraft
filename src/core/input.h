#pragma once

#ifndef INPUT_H
#define INPUT_H
#include <psxpad.h>

#define PAD_SECTIONS 2
#define PAD_SECTION_SIZE 34

typedef struct {
    char pad_buffer[PAD_SECTIONS][PAD_SECTION_SIZE];
    PADTYPE* pad;
} Input;

void initInput(Input* input);

#endif //INPUT_H
