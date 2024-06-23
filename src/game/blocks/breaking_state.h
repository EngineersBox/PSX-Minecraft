#pragma once

#ifndef _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_
#define _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_

#include <psxgte.h>
#include <stdbool.h>

#include "../../math/vector.h"
#include "../../util/inttypes.h"
#include "../../render/render_context.h"
#include "../items/item.h"
#include "block.h"

typedef struct BreakingState {
    /**
     * @brief Amount of ticks in fixed-point format
     *        required to break the block.
     */
    fixedi32 ticks_precise;
    /**
     * @brief Amount of ticks in fixed-point format
     *        per stage of the breaking animation.
     */
    fixedi32 ticks_per_stage;
    /**
     * @brief How many ticks in fixed-point format
     *        we have progressed through so far in
     *        breaking the block.
     *
     * @note Using @code ticks_so_far@endcode and
     *       dividing it by @code ticks_per_stage@endcode,
     *       you can get the index into the breaking
     *       texture at the current point.
     */
    fixedi32 ticks_so_far;
    /**
     * @brief World position of block being broken, this is
     *        0 in all axis when @code block@endcode is
     *        @code NULL@endcode.
     */
    VECTOR position;
    /**
     * @brief @code NULL@endcode == not breaking anything
     */
    IBlock* block;
} BreakingState;

extern const RECT breaking_texture_offscreen;

#define breakingStateReset(state) ({ \
    (state) = (BreakingState) { \
        .ticks_precise = 0, \
        .ticks_per_stage = 0, \
        .ticks_so_far = 0, \
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

/**
 * @brief Updates the off-screen render target texture for each visible face
 *        based on the current breaking progress. If the @code block@endcode field
 *        is @code NULL@endcode then we do nothing. Taking the modulo of @code ticks_so_far@endcode
 *        and @code ticks_per_stage@endcode and comparing against the previous
 *        (@code max(ticks_so_far - ONE, 0))@endcode modulo value, we can determine
 *        if we need to update the render target. If so, the visible faces of
 *        the block are determined via bitset, then for each visible one, the
 *        texture is blitted to the relevant multiple of 16 section of the off-screen
 *        render target and operations are sorted into the ordering-table to
 *        write the breaking texture for the current progress on top of the
 *        render target with 50% F + 50% B blending.
 * @param state State instance
 * @param ctx Render context to sort rendering operations into
 */
void breakingStateUpdateRenderTarget(const BreakingState* state, RenderContext* ctx);

#endif // _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_