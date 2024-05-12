#pragma once

#ifndef PSX_MINECRAFT_PREPROCESSOR_H
#define PSX_MINECRAFT_PREPROCESSOR_H

// ==== META MACROS ====

#define GLUE(x,y) x##y

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

// ==== MARKERS ====

#define __ALLOC_CALL(...) __attribute__((malloc, __VA_ARGS__))

#ifdef GNU_VERSION_10
    #define ALLOC_CALL(destructor, idx) __ALLOC_CALL(malloc(destructor,idx))
#else
    #define ALLOC_CALL(destructor, idx) __ALLOC_CALL({})
#endif

#define UNIMPLEMENTED __attribute__((unavailable("Unimplemented function/method")))

// Forward declaration
#define FWD_DECL

// ==== PRINTING ====

#define INT8_BIN_PATTERN "%c%c%c%c%c%c%c%c"
#define INT8_BIN_LAYOUT(i) \
    ((i) & 0x80 ? '1' : '0'), \
    ((i) & 0x40 ? '1' : '0'), \
    ((i) & 0x20 ? '1' : '0'), \
    ((i) & 0x10 ? '1' : '0'), \
    ((i) & 0x08 ? '1' : '0'), \
    ((i) & 0x04 ? '1' : '0'), \
    ((i) & 0x02 ? '1' : '0'), \
    ((i) & 0x01 ? '1' : '0')

#define INT16_BIN_PATTERN INT8_BIN_PATTERN INT8_BIN_PATTERN
#define INT32_BIN_PATTERN INT16_BIN_PATTERN INT16_BIN_PATTERN
#define INT16_BIN_LAYOUT(i) INT8_BIN_LAYOUT((i) >> 8), INT8_BIN_LAYOUT(i)
#define INT32_BIN_LAYOUT(i) INT16_BIN_LAYOUT((i) >> 16), INT16_BIN_LAYOUT(i)

#endif // PSX_MINECRAFT_PREPROCESSOR_H
