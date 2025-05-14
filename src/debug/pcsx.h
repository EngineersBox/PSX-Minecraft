/**
* Kernel checking and memory sanitiser
* provided by PCSX-Redux vi Mips API:
* https://pcsx-redux.consoledev.net/mips_api/#memory-sanitizer
*
* MIT License
* 
* Copyright (c) 2020 PCSX-Redux authors
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*/

#pragma once

#ifndef _DEBUG__ASAN_H_
#define _DEBUG__ASAN_H_

#include <stdint.h>
#include <stddef.h>
#include "debug_defines.h"

// ==== UTILS ====

#if isDebugTagEnabled(PSXC_UTILS)
// Print ASCII character with code c to console/stdout.
void pcsx_putc(int c);
// Break execution (Pause emulation).
void pcsx_debugbreak();
// Executes Lua function at PCSX.execSlots[slot]. The slot value can be between 1 and 255.
// If no Lua function exists within a slot, then this behaves the same as pcsx_debugbreak().
void pcsx_execSlot(uint8_t slot);
// Exit emulator and forward code as exit code.
void pcsx_exit(int code);
// Create a UI dialog displaying msg
void pcsx_message(const char* msg);
#endif

// ==== KERNEL CHECKING ====

#if isDebugTagEnabled(PCSX_KERNEL_CHECKER)
// Enable or disable kernel checking.
void pcsx_checkKernel(int enable);
// Returns truthy if kernel checking is enabled.
int pcsx_isCheckingKernel();
// Returns 1 if code is running in PCSX-Redux
int pcsx_present();
#endif

// === MEMORY SANITISER ====

#if isDebugTagEnabled(PCSX_ASAN)
// Initialize memory sanitizer system.
void pcsx_initMsan();
// Reset memory sanitizer system.
void pcsx_resetMsan();
// Allocate memory with memory sanitizer.
void* pcsx_msanAlloc(size_t size);
// Allocate memory with memory sanitizer.
void* pcsx_msanCalloc(size_t num, size_t size);
// Free memory with memory sanitizer.
void pcsx_msanFree(void* ptr);
// Reallocate memory with memory sanitizer.
void* pcsx_msanRealloc(void* ptr, size_t size);
// Internal function
void pcsx_msanSetChainPtr(void* headerAddr, void* ptrToNext, uint32_t wordCount);
// Internal function
void* pcsx_msanGetChainPtr(void* headerAddr);
// Internal GCC constructor function
void __msan_init();
#endif

#endif // _DEBUG__ASAN_H_
