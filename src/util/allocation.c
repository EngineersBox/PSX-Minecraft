#include "allocation.h"

#include <assert.h>

__attribute__((always_inline))
inline void* checked_malloc(const size_t size) {
    void* mem = malloc(size);
    assert(mem != NULL);
    return mem;
}

__attribute__((always_inline))
inline void* checked_calloc(const size_t num, const size_t size) {
    void* mem = calloc(num, size);
    assert(mem != NULL);
    return mem;
}

__attribute__((always_inline))
inline void* checked_realloc(void *ptr, const size_t size) {
    void* mem = realloc(ptr, size);
    assert(mem != NULL);
    return mem;
}