#pragma once

#ifndef PSX_MINECRAFT_PREPROCESSOR_H
#define PSX_MINECRAFT_PREPROCESSOR_H

// See: https://gustedt.wordpress.com/2011/03/18/statement-unrolling-with-p99_for/
#define P99_PROTECT(...) __VA_ARGS__

#define P99_ENUM_ENTRY(x) x,
#define P99_STRING_ARRAY_INDEX(x) [x] = #x,

#if defined(__GNUC__) \
    && __GNUC__ >= 10    \
    && (__GNUC__ > 10 || (__GNUC__ >= 0 && __GNUC_MINOR__ >= 0)) \
    && defined(__GNUC_PATCHLEVEL__)
#define GNU_VERSION_10
#endif

#define __ALLOC_CALL(...) __attribute__((malloc, __VA_ARGS__))

#ifdef GNU_VERSION_10
#define ALLOC_CALL(destructor, idx) __ALLOC_CALL(malloc(destructor,idx))
#else
#define ALLOC_CALL(destructor, idx) __ALLOCATOR({})
#endif

#endif // PSX_MINECRAFT_PREPROCESSOR_H
