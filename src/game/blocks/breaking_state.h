#pragma once

#ifndef _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_
#define _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_

#include <psxgte.h>
#include <stdbool.h>

#include "../../math/vector.h"
#include "../../util/inttypes.h"
#include "../items/item.h"
#include "block.h"

typedef struct BreakingState {
    /**
     * @brief Value greater than 0 indicates being broken
     *        and a value of 0 indicates broken iff
     *        @code block@endcode is not @code NULL@endcode
     */
    u32 ticks_left;
    /**
     * @brief Number of ticks that each breaking texture
     *        stage should take.
     */
    u32 ticks_per_stage;
    /**
     * @brief World position of block being broken, this is
     *        0 in all axis when @code block@endcode is
     *        @code NULL@endcode
     */
    VECTOR position;
    /**
     * @brief @code NULL@endcode == not breaking anything
     */
    IBlock* block;
} BreakingState;

#define breakingStateReset(state) ({ \
    (state) = (BreakingState) { \
        .ticks_left = 0, \
        .ticks_per_stage = 0, \
        .position = vec3_i32_all(0), \
        .block = NULL \
    }; \
})

/**
 * @brief Calculates tick count to break the block currently in state and
 *        then sets the @code ticks_left@endcode field accordingly.
 *
 * @see https://minecraft.wiki/w/Breaking
 *
 * @param state State instance
 * @param held_item Item in current hotbar slot (can be @code NULL@endcode)
 * @param in_water Whether the player is currently in water
 * @param on_ground Whether the player is currently on the ground
 */
void breakingStateCalculateTicks(BreakingState* state, const IItem* held_item, bool in_water, bool on_ground);

#endif // _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_