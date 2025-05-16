#include "core/engine.h"
#include "core/app_logic.h"
#include "game/minecraft.h"

static AppLogic app_logic;
static Minecraft* minecraft;

int main() {
    app_logic = DYN_LIT(Minecraft, AppLogic, {});
    minecraft = (Minecraft*) &app_logic;
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

// int main() {
//     u32* array = malloc(sizeof(u32));
//     memset(array, '\0', 1);
//     // int byte_value = '\0';
//     // asm ("swr %1, 0(%0)"
//     //     :
//     //     : "r" (array), "r" (byte_value));
//     free(array);
//     return 0;
// }
