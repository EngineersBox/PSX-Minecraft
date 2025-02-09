#pragma once

#ifndef _PSXMC__UTIL__DEBOUNCE_H_
#define _PSXMC__UTIL__DEBOUNCE_H_

#include <stdbool.h>

#include "../hardware/counters.h"

bool debounce(Timestamp* debounce_field, Timestamp duration);

#define debounceIsExpired(field, duration) (time_ms - (field) >= (duration))
#define debounceResetDelay(field) ((field) = time_ms)
#define debounceResetNoDelay(field) ((field) = 0)

#endif // _PSXMC__UTIL__DEBOUNCE_H_
