#pragma once

#ifndef PSXMC_PREPROCESSOR_H
#define PSXMC_PREPROCESSOR_H

#include <stdio.h>
#include "../core/std/stdlib.h"
#include <assert.h>
#include <metalang99.h>
#include <inttypes.h>

// #include "../logging/logging.h"
#include "../debug/debug_defines.h"

// ==== META MACROS ====

#define XSTRINGIFY(s) STRINGIFY(s)
#define STRINGIFY(s) #s

#define GLUE(x,y) x##y

// See: https://gustedt.wordpress.com/2011/03/18/statement-unrolling-with-p99_for/
#define P99_PROTECT(...) __VA_ARGS__

#define P99_ENUM_ENTRY(x) x,
#define P99_STRING_ARRAY_INDEX(x) [x] = #x,

// Detect GCC only to ensure compilation elements such
// as attributes that are GCC only or have alternative
// formats that only GCC supports, can be detected.
#if !defined(__clang__) && defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
    #define GNU_VERSION (__GNU_VERSION * 10000 + __GNU_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
    #define GNU_GNU_VERSION 0
#endif

// ==== MARKERS ====

#define __ALLOC_CALL(...) __attribute__((malloc, ##__VA_ARGS__))
#if isDebugFlagEnabled(PCSX_ASAN)
    #define ALLOC_CALL(destructor, idx)
#elif GNU_VERSION >=100000
    #define ALLOC_CALL(destructor, idx) __ALLOC_CALL(malloc(destructor,idx))
#else
    #define ALLOC_CALL(destructor, idx) __ALLOC_CALL()
#endif

#define UNAVAILABLE __attribute__((unavailable("Unavailable function/method")))
#define UNUSED __attribute__((unused))
#define MAYBE_UNUSED __attribute__((unused))
#define ALIAS(name) __attribute__((alias(name)))
#define ASM_ALIAS(name) asm(name)
#define MAY_ALIAS __attribute__((may_alias))
#define INLINE __attribute__((always_inline)) inline
#define WEAK __attribute__((weak))
#define PACKED __attribute__((packed))
#define FALLTHROUGH __attribute__((fallthrough))
#define NO_RETURN __attribute__((noreturn))
#define COUNTED_BY(field) __attribute__((counted_by(field)))

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

#define VEC_PATTERN "(%d,%d,%d)"
#define VEC_LAYOUT(v) (v).vx, (v).vy, (v).vz
#define VEC_PTR_LAYOUT(v) (v)->vx, (v)->vy, (v)->vz

#define MAT_PATTERN "%d, %d, %d | %d,\n%d, %d, %d | %d,\n%d, %d, %d | %d\n"
#define MAT_LAYOUT(_m) (_m).m[0][0], (_m).m[0][1], (_m).m[0][2], (_m).t[0], \
    (_m).m[1][0], (_m).m[1][1], (_m).m[1][2], (_m).t[1], \
    (_m).m[2][0], (_m).m[2][1], (_m).m[2][2], (_m).t[2]
#define MAT_PTR_LAYOUT(_m) (_m)->m[0][0], (_m)->m[0][1], (_m)->m[0][2], (_m)->t[0], \
    (_m)->m[1][0], (_m)->m[1][1], (_m)->m[1][2], (_m)->t[1], \
    (_m)->m[2][0], (_m)->m[2][1], (_m)->m[2][2], (_m)->t[2]

#define INT64_PATTERN "0x%08x%08x"
#define INT64_LAYOUT(v) (u32) (((u64)(v)) >> 32), (u32) (v)

// ==== ENUM CONSTRUCTORS ====
// #define ENUM_ENTRY(name) ML99_choice(v(ENUM_ENTRY), v(name))
// #define ENUM_ENTRY_ORD(name, id) ML99_choice(v(ENUM_ENTRY_ORD), v(name), v(id))
//
// #define createEnumEntry_ENUM_ENTRY_IMPL(name) v(name)
// #define createEnumEntry_ENUM_ENTRY_ORD_IMPL(name, id) v(name = id)
//
// #define createEnumEntry_IMPL(entry) ML99_match(v(entry), v(createEnumEntry_))
// #define createEnumEntry_ARITY 1
//
// #define createEnumEntry_IMPL(entry) ML99_match(v(entry), v(createEnumEntry_))
// #define createEnumEntry_ARITY 1
//
// #define enumEntryExtractName_IMPL(choice) ML99_stringify(ML99_tupleGet(0)(ML99_tuple(ML99_choiceData(v(choice)))))
// #define enumEntryExtractName_ARITY 1
//
// #define ENUM_ENTRIES(...) ML99_LIST_EVAL_COMMA_SEP(
//     ML99_listMap(
//         v(createEnumEntry),
//         ML99_list(__VA_ARGS__)
//     )
// )
// #define ENUM_ENTRY_NAMES(...) ML99_LIST_EVAL_COMMA_SEP(
//     ML99_listMap(
//         v(enumEntryExtractName),
//         ML99_list(__VA_ARGS__)
//     )
// )

#define ENUM_ENTRY(name) ML99_choice(v(ENUM_ENTRY), v(name))
#define ENUM_ENTRY_ORD(name, id) ML99_choice(v(ENUM_ENTRY_ORD), v(name), v(id))

#define constructEnum_ENUM_ENTRY_IMPL(name) v(name,)
#define constructEnum_ENUM_ENTRY_ORD_IMPL(name, id) v(name = id,)

#define enumConstruct(entry) ML99_EVAL(ML99_match(entry, v(constructEnum_)))
#define enumExtractNames(entry) ML99_EVAL(ML99_stringify( \
    ML99_tupleGet(0)( \
        ML99_tuple(ML99_choiceData(entry)) \
    ) \
))

#define count_ENUM_ENTRY_IMPL(name) v(1 +)
#define count_ENUM_ENTRY_ORD_IMPL(name, id) count_ENUM_ENTRY_IMPL(name)

#define enumCount(entry) ML99_EVAL(ML99_match(entry, v(count_)))

// ==== PRIMITIVE OPERANDS ====

#define setTPageSemiTrans_T(p, st) \
    (p) &= ~((u32) 3 << 5); \
    (p) |= (st) << 5
#define setTPageSemiTrans(p, st) setTPageSemiTrans_T((p)->code[0], st)

// ==== DEFER OPERATION IN SCOPE ====

#if defined(__clang__)
// Source: https://stackoverflow.com/questions/24959440/rewrite-gcc-cleanup-macro-with-nested-function-for-clang
typedef void (^__cleanup_block)(void);
static inline void __do_cleanup(__cleanup_block* b) { (*b)(); }
#define __DEFER__(V) __cleanup_block __attribute__((cleanup(__do_cleanup))) V = ^
#define __DEFER_(N) __DEFER__(__DEFER_VARIABLE_ ## N)
#define __DEFER(N) __DEFER_(N)
#define defer __DEFER(__COUNTER__)
#elif defined(__GNUC__)
// Source: https://gustedt.wordpress.com/2025/01/06/simple-defer-ready-to-use/
#define __block
#define __DEFER__(F, V) \
    auto void F(int*); \
    __attribute__((cleanup(F))) int V; \
    auto void F(int*)
#define __DEFER_(N) __DEFER__(__DEFER_FUNCTION_ ## N, __DEFER_VARIABLE_ ## N)
#define __DEFER(N) __DEFER_(N)
#define defer __DEFER(__COUNTER__)
#elif
#error defer macro not supported as nested functions not supported with this compiler
#endif

#endif // PSXMC_PREPROCESSOR_H
