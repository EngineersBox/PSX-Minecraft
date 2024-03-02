#pragma once

#ifndef PSX_MINECRAFT_PREPROCESSOR_H
#define PSX_MINECRAFT_PREPROCESSOR_H

// See: https://gustedt.wordpress.com/2011/03/18/statement-unrolling-with-p99_for/
#define P99_PROTECT(...) __VA_ARGS__

#define P99_ENUM_ENTRY(x) x,
#define P99_STRING_ARRAY_INDEX(x) [x] = #x,

#endif // PSX_MINECRAFT_PREPROCESSOR_H
