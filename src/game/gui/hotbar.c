#include "hotbar.h"

#include <interface99_extensions.h>

#include "../../ui/components/background.h"

void hotbarInit(Hotbar* hotbar) {
    uiInit(&hotbar->ui);
    hotbar->ui.active = true;
    hotbar->ui.title = "hotbar";
    hotbar->slots = NULL;
    cvector_init(hotbar->slots, HOTBAR_SLOT_COUNT, NULL);
    cvector_push_back(hotbar->ui.components, (IUIComponent) {});
    IUIComponent* component = &hotbar->ui.components[cvector_size(hotbar->ui.components)];
    UIBackground* background = (UIBackground*) malloc(sizeof(UIBackground));
    background->component.dimensions = (DVECTOR) {
        .vx = HOTBAR_WIDTH,
        .vy = HOTBAR_HEIGHT
    };
    background->component.position = (DVECTOR) {
        .vx = CENTRE_X - (HOTBAR_WIDTH / 2),
        .vy = SCREEN_YRES - HOTBAR_HEIGHT - 1
    };
    background->texture_coords = (DVECTOR) {
        .vx = 576,
        .vy = 1
    };
    background->texture_width = (DVECTOR) {
        .vx = HOTBAR_WIDTH,
        .vy = HOTBAR_HEIGHT
    };
    DYN_PTR(component, UIBackground, IUIComponent, background);
}

void hotbarLoadTexture(VSelf) __attribute__((alias("Hotbar_loadTexture")));
void Hotbar_loadTexture(VSelf) {}

void hotbarFreeTexture(VSelf) __attribute__((alias("Hotbar_freeTexture")));
void Hotbar_freeTexture(VSelf) {}