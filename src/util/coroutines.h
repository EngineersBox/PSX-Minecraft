/*
 * Support for coroutines in C as laid out within
 * the following article: https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html.
 * Is this hacky? yes. But is it cool? hell yeah.
 *
 * Usage rules:
 * 1. Routine body must be surrounded with coroutineBegin() and coroutineEnd()
 * 2. Declare all local variables static if they need to be preserved across a coroutineReturn(...)
 * 3. Never put a coroutineReturn(...) within an explicit switch statement
 * 4. Never put more than one coroutineReturn(...) statements on the same line
 *
 * Example:
 * int function() {
 *     static int i;
 *     coroutineBegin();
 *     for (i = 0; i < 10; i++) {
 *         coroutineReturn(i);
 *     }
 *     coroutineEnd();
 *     return i;
 * }
 */

#pragma once

#ifndef PSX_MINECRAFT_COROUTINES_H
#define PSX_MINECRAFT_COROUTINES_H

#define coroutineBegin() static int __cr_state = 0; switch (__cr_state) { case 0:
#define coroutineReturn(x) do { __cr_state = __LINE__; return x; \
                                case __LINE__:; } while (0)
#define coroutineEnd() }

#endif // PSX_MINECRAFT_COROUTINES_H
