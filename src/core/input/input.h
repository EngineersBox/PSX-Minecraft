#pragma once

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <interface99.h>
#include <psxpad.h>

#include "../../structure/cvector.h"

#define PAD_SECTIONS 2
#define PAD_SECTION_SIZE 34

typedef struct Input Input;

typedef bool (*InputHandler)(const Input* input, void* ctx);
typedef void (*InputHandlerDestroy)(Input* input, void* ctx);
typedef struct {
    void* ctx;
    InputHandler input_handler;
    InputHandlerDestroy input_handler_destroy;
} InputHandlerVTable;

typedef struct Input {
    char pad_buffer[PAD_SECTIONS][PAD_SECTION_SIZE];
    PADTYPE* pad;
    InputHandlerVTable* in_focus;
    cvector(InputHandlerVTable) handlers;
} Input;

#define IInputHandler_IFACE \
    vfunc(void, registerInputHandler, VSelf, Input* input, void* ctx)

interface(IInputHandler);

void inputInit(Input* input);
void inputUpdate(Input* input);

#define inputAddHandler(input, handler) cvector_push_back((input)->handlers, (InputHandlerVTable)(handler))
#define isPressed(input_pad, pad_button) (!((input_pad)->btn & (pad_button)))

#endif //INPUT_H
