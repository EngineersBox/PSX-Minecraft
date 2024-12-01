#pragma once

#ifndef PSXMC_BINDINGS_H
#define PSXMC_BINDINGS_H

#include <psxpad.h>

// Movement
extern PadButton BINDING_MOVE_FORWARD;
extern PadButton BINDING_MOVE_BACKWARD;
extern PadButton BINDING_MOVE_LEFT;
extern PadButton BINDING_MOVE_RIGHT;

// Look
extern PadButton BINDING_LOOK_UP;
extern PadButton BINDING_LOOK_DOWN;
extern PadButton BINDING_LOOK_LEFT;
extern PadButton BINDING_LOOK_RIGHT;

extern PadButton BINDING_JUMP;
extern PadButton BINDING_SNEAK;

// Utilise held item
extern PadButton BINDING_ATTACK;
extern PadButton BINDING_USE;

// Interaction
extern PadButton BINDING_DROP_ITEM;
extern PadButton BINDING_OPEN_INVENTORY;
extern PadButton BINDING_CURSOR_CLICK;
extern PadButton BINDING_PREVIOUS;
extern PadButton BINDING_NEXT;

// Game/system state transitions
extern PadButton BINDING_PAUSE;

#endif // PSXMC_BINDINGS_
