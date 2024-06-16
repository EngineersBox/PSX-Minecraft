#pragma once

#ifndef _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_
#define _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_

#include <psxgte.h>
#include <stdbool.h>

#include "../../math/vector.h"
#include "../../util/inttypes.h"
#include "../items/item.h"
#include "block.h"

typedef struct {
    u32 ticks_left;
    VECTOR position;
    IBlock* block; // NULL == not breaking anything
} BreakingState;

#define breakingStateReset(state) ({ \
    (state) = (BreakingState) { \
        .ticks_left = 0, \
        .position = vec3_i32_all(0), \
        .block = NULL \
    }; \
})

void breakingStateCalculateTicksLeft(BreakingState* state, const IItem* held_item, bool in_water, bool on_ground);

#endif // _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_