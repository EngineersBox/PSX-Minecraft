#pragma once

#ifndef PSXMC_LOGGING_H
#define PSXMC_LOGGING_H

#include <stdio.h>

#include "../debug/debug_defines.h"
#include "../util/preprocessor.h"

#if isDebugEnabled()
#define DEBUG_LOG(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEBUG_LOG(x, ...) ({})
#endif

NO_RETURN void errorAbort(const char* fmt, ...);

/**
 * @brief Indicates not yet implemented functionality,
 *        implying that it will be implemented later
 * @param msg Intention of the functionality to be implemented
 */
#define TODO(msg) ({ \
    printf( \
        "[TODO :: Start]\nLocation: %s @ %s:%d\n"msg"\n[TODO :: End]\n", \
        __func__, \
        __FILE__, \
        __LINE__ \
    ); \
})
/**
 * @brief Indicates not yet implemented functionality,
 *        but does not claim it will be implemented later
 */
#define UNIMPLEMENTED() errorAbort( \
    "[ERROR] Invoked unimplemented function %s @ %s:%d\n", \
    __func__, \
    __FILE__, \
    __LINE__ \
)

#endif // PSXMC_LOGGING_H
