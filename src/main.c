#include <interface99_extensions.h>

#include "core/engine.h"
#include "core/app_logic.h"
#include "game/minecraft.h"

static AppLogic app_logic;
static Minecraft* minecraft;

int main() {
    app_logic = DYN_LIT(Minecraft, AppLogic, {});
    minecraft = VCAST(Minecraft*, app_logic);
    Engine engine = (Engine) {
        .app_logic =  &app_logic,
        .running = true,
        .target_fps = 60,
        .target_tps = 20
    };
    engineInit(&engine, NULL);
    engineRun(&engine);
    return 0;
}
// #include <assert.h>
// #include <stdint.h>
// #include <stddef.h>
// #include <stdio.h>
//
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-variable"
//
// void pcsx_initMsan() { *((volatile char* const)0x1f802089) = 0; }
//
// void* pcsx_msanAlloc(size_t size) {
//     register uint32_t a0 asm("a0") = (uint32_t) size;
//     return *((void* volatile* const)0x1f80208c);
// }
//
// void pcsx_msanFree(void* ptr) { *((void* volatile* const)0x1f80208c) = ptr; }
//
// #pragma GCC diagnostic pop
//
// int main() {
//     pcsx_initMsan();
//     register int32_t* array = pcsx_msanAlloc(sizeof(int32_t));
//     register int32_t byte_value = 0x11223344;
//     // Should succeed
//     asm ("swl %0, 1(%1)"
//         :
//         : "r" (byte_value), "r" (array));
//     printf("SWL suceeded\n");
//     register volatile int32_t result = 0;
//     // Should fail
//     asm ("lwr %0, 0(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWR 0: 0x%x\n", result);
//     result = 0;
//     // Should fail
//     asm ("lwr %0, 1(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWR 1: 0x%x\n", result);
//     result = 0;
//     // Should succeed
//     asm ("lwr %0, 2(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWR 2: 0x%x\n", result);
//     result = 0;
//     // Should succeed
//     asm ("lwr %0, 3(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWR 3: 0x%x\n", result);
//     result = 0;
//     // Should succeed
//     asm ("lwl %0, 0(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWL 0: 0x%x\n", result);
//     result = 0;
//     // Should succeed
//     asm ("lwl %0, 1(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWL 1: 0x%x\n", result);
//     result = 0;
//     // Should fail
//     asm ("lwl %0, 2(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWL 2: 0x%x\n", result);
//     result = 0;
//     // Should fail
//     asm ("lwl %0, 3(%1)"
//         : "+r"(result)
//         : "r"(array));
//     printf("LWL 3: 0x%x\n", result);
//     pcsx_msanFree(array);
//     return 0;
// }
