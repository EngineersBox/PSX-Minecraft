#include "logging.h"

#include "../core/std/stdlib.h"
#include <string.h>

__attribute__((
    noreturn,
    format(printf, 1, 2)
))
inline void errorAbort(const char* fmt, ...) {
    char* str = calloc(strlen(fmt), sizeof(char));
    va_list(args);
    va_start(args, fmt);
    vsprintf(str, fmt, args);
    va_end(args);
    printf(str);
    abort();
}
