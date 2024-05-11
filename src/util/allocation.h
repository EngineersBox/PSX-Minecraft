#pragma once

#ifndef _PSX_MINECRAFT__UTIL__ALLOCATION_H_
#define _PSX_MINECRAFT__UTIL__ALLOCATION_H_

#include <stdlib.h>

void* checked_malloc(const size_t size);
void* checked_calloc(const size_t num, const size_t size);
void* checked_realloc(void *ptr, const size_t size);

#endif // _PSX_MINECRAFT__UTIL__ALLOCATION_H_