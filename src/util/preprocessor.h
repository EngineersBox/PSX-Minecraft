#pragma once

#ifndef PSX_MINECRAFT_PREPROCESSOR_H
#define PSX_MINECRAFT_PREPROCESSOR_H

#include <metalang99.h>

// ==== META MACROS ====

#define GLUE(x,y) x##y

// See: https://gustedt.wordpress.com/2011/03/18/statement-unrolling-with-p99_for/
#define P99_PROTECT(...) __VA_ARGS__

#define P99_ENUM_ENTRY(x) x,
#define P99_STRING_ARRAY_INDEX(x) [x] = #x,

#if defined(__GNUC__) \
    && __GNUC__ >= 10 \
    || ( \
        defined(__GNUC_PATCHLEVEL__) \
        && (__GNUC__ > 10 || (__GNUC__ >= 0 && __GNUC_MINOR__ >= 0)) \
    )
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
#define UNUSED __attribute__((unused))
#define ALIAS(name) __attribute__((alias(name)))
#define INLINE __attribute((always_inline)) inline

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

#define INT64_PATTERN "%d_%d"
#define INT64_LAYOUT(v) (i32) ((v) >> 32), (i32) ((v) & (UINT32_MAX - 1))

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

#define constructEnum_ENUM_ENTRY_IMPL(name) v(name)
#define constructEnum_ENUM_ENTRY_ORD_IMPL(name, id) v(name = id)

#define enumConstruct(entry) ML99_EVAL(ML99_match(entry, v(constructEnum_)))
#define enumExtractNames(entry) ML99_EVAL(ML99_stringify( \
    ML99_tupleGet(0)( \
        ML99_tuple(ML99_choiceData(entry)) \
    ) \
))

#endif // PSX_MINECRAFT_PREPROCESSOR_H
