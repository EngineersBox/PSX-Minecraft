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

#include "../util/inttypes.h"
#include "../util/preprocessor.h"
#include "debug_defines.h"

// ==== UTILS ====

#if isDebugTagEnabled(PSXC_UTILS)
// Print ASCII character with code c to console/stdout.
static __inline__ void pcsx_putc(int c) { *((volatile char* const)0x1f802080) = c; }
// Break execution (Pause emulation).
static __inline__ void pcsx_debugbreak() { *((volatile char* const)0x1f802081) = 0; }
// Executes Lua function at PCSX.execSlots[slot]. The slot value can be between 1 and 255.
// If no Lua function exists within a slot, then this behaves the same as pcsx_debugbreak().
static __inline__ void pcsx_execSlot(uint8_t slot) { *((volatile uint8_t* const)0x1f802081) = slot; }
// Exit emulator and forward code as exit code.
static __inline__ void pcsx_exit(int code) { *((volatile int16_t* const)0x1f802082) = code; }
// Create a UI dialog displaying msg
static __inline__ void pcsx_message(const char* msg) { *((volatile char** const)0x1f802084) = msg; }
#endif

// ==== KERNEL CHECKING ====

#if isDebugTagEnabled(PCSX_KERNEL_CHECKER)
// Enable or disable kernel checking.
static __inline__ void pcsx_checkKernel(int enable) { *((volatile char*)0x1f802088) = enable; }
// Returns truthy if kernel checking is enabled.
static __inline__ int pcsx_isCheckingKernel() { return *((volatile char* const)0x1f802088) != 0; }
// Returns 1 if code is running in PCSX-Redux
static __inline__ int pcsx_present() { return *((volatile uint32_t* const)0x1f802080) == 0x58534350; }
#endif

// === MEMORY SANITISER ====

#if isDebugTagEnabled(PCSX_ASAN)
// Initialize memory sanitizer system.
static __inline__ void pcsx_initMsan() { *((volatile char* const)0x1f802089) = 0; }
// Reset memory sanitizer system.
static __inline__ void pcsx_resetMsan() { *((volatile char* const)0x1f802089) = 1; }
// Allocate memory with memory sanitizer.
static __inline__ void* pcsx_msanAlloc(uint32_t size) {
    MAYBE_UNUSED register uint32_t a0 asm("a0") = size;
    return *((void* volatile* const)0x1f80208c);
}
// Allocate memory with memory sanitizer.
static __inline__ void* pcsx__msanCalloc(uint32_t num, uint32_t size) {
    return pcsx_msanAlloc(num * size);
}
// Free memory with memory sanitizer.
static __inline__ void pcsx_msanFree(void* ptr) { *((void* volatile* const)0x1f80208c) = ptr; }
// Reallocate memory with memory sanitizer.
static __inline__ void* pcsx_msanRealloc(void* ptr, uint32_t size) {
    MAYBE_UNUSED register void* a0 asm("a0") = ptr;
    MAYBE_UNUSED register uint32_t a1 asm("a1") = size;
    return *((void* volatile* const)0x1f802090);
}
// Internal function
static __inline__ void pcsx_msanSetChainPtr(void* headerAddr, void* ptrToNext, uint32_t wordCount) {
    MAYBE_UNUSED register void* a0 asm("a0") = ptrToNext;
    MAYBE_UNUSED register uint32_t a1 asm("a1") = wordCount;
    *((void* volatile* const)0x1f802094) = headerAddr;
}
// Internal function
static __inline__ void* pcsx_msanGetChainPtr(void* headerAddr) {
    MAYBE_UNUSED register void* a0 asm("a0") = headerAddr;
    return *((void* volatile* const)0x1f802094);
}
#endif

#endif // _DEBUG__ASAN_H_
