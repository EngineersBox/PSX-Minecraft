/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Evan Teran
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#ifndef CVECTOR_UTILS_H_
#define CVECTOR_UTILS_H_

/**
 * @brief cvector_for_each_in - for header to iterate over vector each element's address
 * @param it - iterator of type pointer to vector element
 * @param vec - the vector
 * @return void
 */
#define cvector_for_each_in(it, vec) \
    for (it = cvector_begin(vec); it < cvector_end(vec); it++)

/**
 * @brief cvector_for_each - call function func on each element of the vector
 * @param vec - the vector
 * @param func - function to be called on each element that takes each element as argument
 * @return void
 */
#define cvector_for_each(vec, func)                   \
    do {                                              \
        if ((vec) && (func) != NULL) {                \
            size_t i;                                 \
            for (i = 0; i < cvector_size(vec); i++) { \
                func((vec)[i]);                       \
            }                                         \
        }                                             \
    } while (0)

/**
 * @brief cvector_free_each_and_free - calls `free_func` on each element
 * contained in the vector and then destroys the vector itself
 * @param vec - the vector
 * @param free_func - function used to free each element in the vector with
 * one parameter which is the element to be freed)
 * @return void
 */
#define cvector_free_each_and_free(vec, free_func) \
    do {                                           \
        cvector_for_each((vec), (free_func));      \
        cvector_free(vec);                         \
    } while (0)

#endif /* CVECTOR_UTILS_H_ */
