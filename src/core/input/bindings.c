#include "bindings.h"

__attribute__((weak)) PadButton BINDING_MOVE_FORWARD = PAD_UP;
__attribute__((weak)) PadButton BINDING_MOVE_BACKWARD = PAD_DOWN;
__attribute__((weak)) PadButton BINDING_MOVE_LEFT = PAD_LEFT;
__attribute__((weak)) PadButton BINDING_MOVE_RIGHT = PAD_RIGHT;

__attribute__((weak)) PadButton BINDING_LOOK_UP = PAD_TRIANGLE;
__attribute__((weak)) PadButton BINDING_LOOK_DOWN = PAD_CROSS;
__attribute__((weak)) PadButton BINDING_LOOK_LEFT = PAD_SQUARE;
__attribute__((weak)) PadButton BINDING_LOOK_RIGHT = PAD_CIRCLE;

__attribute__((weak)) PadButton BINDING_JUMP = PAD_R1;
__attribute__((weak)) PadButton BINDING_SNEAK = PAD_L1;

__attribute__((weak)) PadButton BINDING_ATTACK = PAD_R2;
__attribute__((weak)) PadButton BINDING_USE = PAD_L2;

__attribute__((weak)) PadButton BINDING_OPEN_INVENTORY = PAD_START;
// Need to adjust these since they don't feel right for these actions
__attribute__((weak)) PadButton BINDING_PREVIOUS = PAD_L3;
__attribute__((weak)) PadButton BINDING_NEXT = PAD_R3;

__attribute__((weak)) PadButton BINDING_PAUSE = PAD_SELECT;