#pragma once

#ifndef _PSXMC__UTIL__DEBOUNCE_H_
#define _PSXMC__UTIL__DEBOUNCE_H_

#include <stdbool.h>

#include "../hardware/counters.h"

bool debounce(Timestamp* debounce_field, Timestamp duration);

#endif // _PSXMC__UTIL__DEBOUNCE_H_
