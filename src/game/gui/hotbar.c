#include "hotbar.h"

#include <cvector_utils.h>
#include <interface99_extensions.h>

#include "../../ui/components/background.h"

void hotbarInit(Hotbar* hotbar) {
    uiInit(&hotbar->ui);
    hotbar->ui.active = true;
    hotbar->ui.title = "hotbar";
    hotbar->slots = NULL;
    cvector_init(hotbar->slots, HOTBAR_SLOT_COUNT, NULL);
    for (uint8_t i = 0; i < 9; i++) {
        cvector_push_back(hotbar->slots, (Slot) {});
        Slot* slot = &hotbar->slots[i];
        slot->dimensions = HOTBAR_SLOT_DIMS;
        slot->position = hotbarSlotPos(i, 0);
        slot->index = i;
    }
    cvector_push_back(hotbar->ui.components, (IUIComponent) {});
    IUIComponent* component = &hotbar->ui.components[cvector_size(hotbar->ui.components) - 1];
    UIBackground* background = (UIBackground*) malloc(sizeof(UIBackground));
    background->component.position = (DVECTOR) {
        .vx = CENTRE_X - (HOTBAR_WIDTH / 2),
        .vy = SCREEN_YRES - HOTBAR_HEIGHT - 1
    };
    background->component.dimensions = (DVECTOR) {
        .vx = HOTBAR_WIDTH,
        .vy = HOTBAR_HEIGHT
    };
    background->texture_coords = (DVECTOR) {
        .vx = (16 * 16),
        .vy = 0
    };
    background->texture_width = (DVECTOR) {
        .vx = HOTBAR_WIDTH,
        .vy = HOTBAR_HEIGHT
    };
    DYN_PTR(component, UIBackground, IUIComponent, background);
}

void hotbarRenderSlots(const Hotbar* hotbar,  RenderContext* ctx, Transforms* transforms) {
    Slot* slot;
    cvector_for_each_in(slot, hotbar->slots) {
        if (slot->item == NULL) {
            continue;
        }
        Item* item = VCAST(Item*, *slot->item);
        item->position.vx = slot->position.vx;
        item->position.vy = slot->position.vy;
        VCALL_SUPER(*slot->item, Renderable, renderInventory, ctx, transforms);
    }
}

void hotbarLoadTexture(VSelf) __attribute__((alias("Hotbar_loadTexture")));
void Hotbar_loadTexture(VSelf) {}

void hotbarFreeTexture(VSelf) __attribute__((alias("Hotbar_freeTexture")));
void Hotbar_freeTexture(VSelf) {}