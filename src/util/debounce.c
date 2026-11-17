#include "debounce.h"

bool debounceApply(Debounce* state, i32 key) {
    if (state->last_key != key) {
        state->last_time = time_ms - (state->match_delay - state->no_match_delay);
        state->last_key = key;
    }
    if ((time_ms - state->last_time) > state->match_delay) {
        state->last_time = time_ms;
        return true;
    }
    return false;
}
