#pragma once

#ifndef _GAME_SAVE__SAVE_H_
#define _GAME_SAVE__SAVE_H_

#include <psxgte.h>

#include "../../hardware/counters.h"

typedef struct Save {
    VECTOR player_pos;
    Timestamp last_played_at;
    Timestamp created_at;
    u32 seed;
    char* name;
    // TODO: World chunk data? Offsets to them?
    //       What format should these be, and
    //       how do we reference them?
} Save;

#endif // _GAME_SAVE__SAVE_H_
