#include "bindings.h"

__attribute__((weak)) PadButton binding_move_forward = PAD_UP;
__attribute__((weak)) PadButton binding_move_backward = PAD_DOWN;
__attribute__((weak)) PadButton binding_move_left = PAD_LEFT;
__attribute__((weak)) PadButton binding_move_right = PAD_RIGHT;

__attribute__((weak)) PadButton binding_look_up = PAD_TRIANGLE;
__attribute__((weak)) PadButton binding_look_down = PAD_CROSS;
__attribute__((weak)) PadButton binding_look_left = PAD_SQUARE;
__attribute__((weak)) PadButton binding_look_right = PAD_CIRCLE;

__attribute__((weak)) PadButton binding_jump = PAD_R1;
__attribute__((weak)) PadButton binding_sneak = PAD_L1;

__attribute__((weak)) PadButton binding_attack = PAD_R2;
__attribute__((weak)) PadButton binding_use = PAD_L2;

__attribute__((weak)) PadButton binding_open_inventory = PAD_START;
// Need to adjust these since they don't feel right for these actions
__attribute__((weak)) PadButton binding_previous = PAD_L3;
__attribute__((weak)) PadButton binding_next = PAD_R3;

__attribute__((weak)) PadButton binding_pause = PAD_SELECT;