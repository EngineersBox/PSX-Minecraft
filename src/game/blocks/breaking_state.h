#pragma once

#ifndef _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_
#define _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_

#include <psxgte.h>

#include "../../util/inttypes.h"
#include "block.h"

typedef struct {
    u32 current_damage;
    VECTOR position;
    IBlock* block; // NULL == not breaking anything
} BreakingState;

#endif // _PSX_MINECRAFT__GAME_BLOCKS__BREAKING_STATE_H_