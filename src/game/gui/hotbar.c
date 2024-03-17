#include "hotbar.h"

void hotbarInit(Hotbar* hotbar) {
    hotbar->slots = NULL;
    cvector_init(hotbar->slots, HOTBAR_SLOT_COUNT, NULL);
}

void hotbarRender(Hotbar* hotbar, RenderContext* ctx, Transforms* transforms) {
    uiRender(&hotbar->ui, ctx, transforms);
}