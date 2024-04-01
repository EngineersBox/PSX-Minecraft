#pragma once

#ifndef PSX_MINECRAFT_INPUT_HANDLER_H
#define PSX_MINECRAFT_INPUT_HANDLER_H

#include <psxpad.h>
#include <stdbool.h>

typedef bool (*InputHandler)(const PADTYPE* pad, void* ctx);
typedef struct {
    void* ctx;
    InputHandler input_handler;
} ContextualInputHandler;

#endif // PSX_MINECRAFT_INPUT_HANDLER_H
