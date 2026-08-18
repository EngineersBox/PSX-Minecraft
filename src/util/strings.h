#pragma once

#ifndef _UTIL__STRINGS_H_
#define _UTIL__STRINGS_H_

#include <stddef.h>

// Converts snake case into regular, spaced
// and capitalised (on first letter of a word)
// text.
//
// Writes the formatted text to dst, considering
// the minimum of src_len and dst_len characters.
void stringToReadable(const char* src,
                      size_t src_len,
                      char* dst,
                      const size_t dst_len);

#endif // _UTIL__STRINGS_H_
