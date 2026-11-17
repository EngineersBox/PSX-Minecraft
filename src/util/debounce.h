#pragma once

#ifndef _PSXMC__UTIL__DEBOUNCE_H_
#define _PSXMC__UTIL__DEBOUNCE_H_

#include <stdbool.h>

#include "../hardware/counters.h"

typedef struct Debounce {
    Timestamp last_time;
    i32 last_key;
    Timestamp match_delay: 16;
    Timestamp no_match_delay: 16;
} Debounce;

#define debounceInit(_match_delay, _no_match_delay) (Debounce) { \
    .last_time = 0, \
    .last_key = INT32_MAX, \
    .match_delay = (_match_delay), \
    .no_match_delay = (_no_match_delay) \
}; \
_Static_assert((_match_delay) >= (_no_match_delay))

bool debounceApply(Debounce* state, i32 key);

#define debounceIsExpired(field, duration) (time_ms - (field) > (duration))
#define debounceResetDelay(field) ((field) = time_ms)
#define debounceResetNoDelay(field) ((field) = 0)

#endif // _PSXMC__UTIL__DEBOUNCE_H_
