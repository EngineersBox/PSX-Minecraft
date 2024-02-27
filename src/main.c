#include "core/engine.h"
#include "core/app_logic.h"
#include "game/minecraft.h"

AppLogic app_logic;
Minecraft* minecraft;

int main() {
    app_logic = DYN_LIT(Minecraft, AppLogic, {});
    minecraft = (Minecraft*) &app_logic;
    Engine engine = (Engine) {
        .app_logic =  &app_logic,
        .target_fps = 60,
        .target_tps = 20
    };
    engineInit(&engine, NULL);
    engineStart(&engine);
    return 0;
}
