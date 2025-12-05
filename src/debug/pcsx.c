#include "pcsx.h"

// ==== UTILS ====

#if isDebugFlagEnabled(PCSX_UTILS)
// Print ASCII character with code c to console/stdout.
void pcsx_putc(int c) { *((volatile char* const)0x1f802080) = c; }
// Break execution (Pause emulation).
void pcsx_debugbreak() { *((volatile char* const)0x1f802081) = 0; }
// Executes Lua function at PCSX.execSlots[slot]. The slot value can be between 1 and 255.
// If no Lua function exists within a slot, then this behaves the same as pcsx_debugbreak().
void pcsx_execSlot(uint8_t slot) { *((volatile uint8_t* const)0x1f802081) = slot; }
// Exit emulator and forward code as exit code.
void pcsx_exit(int code) { *((volatile int16_t* const)0x1f802082) = code; }
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif
// Create a UI dialog displaying msg
void pcsx_message(const char* msg) { *((volatile char** const)0x1f802084) = msg; }
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

// ==== KERNEL CHECKING ====

#if isDebugFlagEnabled(PCSX_KERNEL_CHECKER)
// Enable or disable kernel checking.
void pcsx_checkKernel(int enable) { *((volatile char*)0x1f802088) = enable; }
// Returns truthy if kernel checking is enabled.
int pcsx_isCheckingKernel() { return *((volatile char* const)0x1f802088) != 0; }
// Returns 1 if code is running in PCSX-Redux
int pcsx_present() { return *((volatile uint32_t* const)0x1f802080) == 0x58534350; }
#endif

// === MEMORY SANITISER ====

#if isDebugFlagEnabled(PCSX_ASAN)
#pragma GCC diagnostic push
// Ignore register ASM vars being ignored
#pragma GCC diagnostic ignored "-Wunused-variable"
// Initialize memory sanitizer system.
void pcsx_initMsan() { *((volatile char* const)0x1f802089) = 0; }
// Reset memory sanitizer system.
void pcsx_resetMsan() { *((volatile char* const)0x1f802089) = 1; }
// Allocate memory with memory sanitizer.
void* pcsx_msanAlloc(size_t size) {
    register uint32_t a0 asm("a0") = (uint32_t) size;
    return *((void* volatile* const)0x1f80208c);
}
// Allocate memory with memory sanitizer.
void* pcsx_msanCalloc(size_t num, size_t size) {
    return pcsx_msanAlloc(num * size);
}
// Free memory with memory sanitizer.
void pcsx_msanFree(void* ptr) { *((void* volatile* const)0x1f80208c) = ptr; }
// Reallocate memory with memory sanitizer.
void* pcsx_msanRealloc(void* ptr, size_t size) {
    register void* a0 asm("a0") = ptr;
    register uint32_t a1 asm("a1") = (uint32_t) size;
    return *((void* volatile* const)0x1f802090);
}
// Internal function
void pcsx_msanSetChainPtr(void* headerAddr, void* ptrToNext, uint32_t wordCount) {
    register void* a0 asm("a0") = ptrToNext;
    register uint32_t a1 asm("a1") = wordCount;
    *((void* volatile* const)0x1f802094) = headerAddr;
}
// Internal function
void* pcsx_msanGetChainPtr(void* headerAddr) {
    register void* a0 asm("a0") = headerAddr;
    return *((void* volatile* const)0x1f802094);
}
#pragma GCC diagnostic pop
__attribute__((constructor)) void __msan_init() {
    pcsx_initMsan();
}
#endif
