#include "debounce.h"

bool debounce(Timestamp* debounce_field, const Timestamp duration) {
    if (time_ms - *debounce_field >= duration) {
        *debounce_field = time_ms;
        return true;
    }
    return false;
}
